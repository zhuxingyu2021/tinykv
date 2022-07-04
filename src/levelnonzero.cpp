# include "levelnonzero.h"
# include <queue>
# include <stack>

LevelNonZero::LevelNonZero(Option &op, Manifest &manifest, int level, Cache *tablecache, Cache *blockcache): Level(),
    option(op),_manifest(manifest), tbl_cache(tablecache), blk_cache(blockcache), _level(level), empty(true){}

LevelNonZero::LevelNonZero(Option &op, Manifest &manifest, int level, Utils::LevelMetaDataType &levelmetadata, Cache *tablecache,
                           Cache *blockcache): Level(), option(op), _manifest(manifest), tbl_cache(tablecache),blk_cache(blockcache),
                           _level(level),empty(false){
    for(auto sst: levelmetadata){
        auto id = sst.first;
        ssts.push_back(new SSTable(op, id, op.DB_PATH + std::to_string(id) + std::string(".sst"),
                                   tablecache, blockcache, sst.second.first, sst.second.second));
    }
}

LevelNonZero::~LevelNonZero() {
    for(auto sst: ssts){
        delete sst;
    }
}

// 从当前Level中获得键key的值并返回。若键key不存在，则is_failed对应的bool变量设置为true，表示查询失败；否则设置为false，表示查询成功。
std::string LevelNonZero::Get(uint64_t key, bool *is_failed) const {
    bool failed;
    std::string val;
    std::shared_lock Lock(level_mutex);
    if(empty){if(*is_failed) *is_failed = true; return "";} // 如果sst文件不存在
    for(auto iter=ssts.begin(); iter!=ssts.end(); iter++){
        auto sst = *iter;
        if((sst->GetMinKey()<=key) && (sst->GetMaxKey()>=key)){
            val = sst->Get(key, &failed);
            if(!failed){if(is_failed) *is_failed=false; return val;}
        }
    }
    if(is_failed) *is_failed=true;
    return "";
}

// 获得当前Level的所有SSTable
const std::vector<SSTable*>& LevelNonZero::GetSSTables() const {
    return ssts;
}

// 清空当前Level
void LevelNonZero::Clear() {
    for(auto sst:ssts){
        // 删除相关cache
        tbl_cache->Evict(sst->GetID());
        blk_cache->Evict(sst->GetID());

        // 删除SSTable文件
        remove(sst->GetPath().c_str());

        delete sst;
    }
    ssts.clear();
}


struct priority_queue_type{
    priority_queue_type(std::pair<uint64_t, std::string> _kv, IterableSSTable* _isst, int _level, uint64_t _id):
        kv(_kv),isst(_isst),level(_level),id(_id) {}
    std::pair<uint64_t, std::string> kv;
    IterableSSTable* isst;
    int level;
    uint64_t id;

    bool operator<(const priority_queue_type& a) const
    {
        return kv.first > a.kv.first; //小顶堆
    }

};

// MajorCompaction
void LevelNonZero::MajorCompaction(Level &last_level) {
    std::vector<SSTable*> new_ssts;
    int cur_sst_id = 0;
    SSTable *cur_sst = new SSTable(option, option.DB_PATH + std::string("level") + std::to_string(_level)+
        std::string(".sst.tmp.") + std::to_string(cur_sst_id),tbl_cache, blk_cache);
    cur_sst->CreateSSTFile();
    new_ssts.push_back(cur_sst);

    const std::vector<SSTable*>& last_level_ssts = last_level.GetSSTables();
    std::vector<IterableSSTable*> last_level_issts;
    last_level_issts.reserve(last_level_ssts.size());
    std::vector<IterableSSTable*> this_level_issts;
    this_level_issts.reserve(ssts.size());

    // 优先级队列，用于多路归并
    std::priority_queue<priority_queue_type> heap;

    std::vector<priority_queue_type> tmp; // 用于合并重复的键值对

    // 多路归并
    // 读取每个文件的第一个值，放入优先级队列中
    for(auto sst:last_level_ssts){
        IterableSSTable* isst = new IterableSSTable(*sst, option);
        last_level_issts.push_back(isst);

        heap.push(priority_queue_type(*(isst->default_iterator), isst, last_level.GetLevel(), sst->GetID()));
        ++isst->default_iterator;
    }
    for(auto sst:ssts){
        IterableSSTable* isst = new IterableSSTable(*sst, option);
        this_level_issts.push_back(isst);

        heap.push(priority_queue_type(*(isst->default_iterator), isst, last_level.GetLevel(), sst->GetID()));
        ++isst->default_iterator;
    }
    while(!heap.empty()){
        // 取出优先级队列的第一个元素
        auto t = heap.top();
        heap.pop();

        if(tmp.empty()) tmp.push_back(t);
        else{
            if(t.kv.first==tmp.back().kv.first) tmp.push_back(t);
            else{
                // 合并重复的键值对
                int newest = 0;
                // 找最新的元素
                for(int i=1; i<tmp.size(); i++){
                    if(tmp[i].level<=tmp[newest].level){
                        if(tmp[i].id>tmp[newest].id){
                            newest=i;
                        }
                    }
                }
                // 把最新的元素更新到文件
                cur_sst->WriteDataBlock(heap.top().kv.first, heap.top().kv.second);
                if(cur_sst->WritenSize()>=option.FILE_SPLIT_SIZE){ // SSTable超出文件大小上限，则创建新SSTable
                    cur_sst->WriteMetaData();

                    cur_sst_id++;
                    cur_sst = new SSTable(option, option.DB_PATH + std::string("level") + std::to_string(_level)+
                                                  std::string(".sst.tmp.") + std::to_string(cur_sst_id),tbl_cache, blk_cache);
                    cur_sst->CreateSSTFile();
                    new_ssts.push_back(cur_sst);
                }
                tmp.clear();
                tmp.push_back(t);
            }
        }

        // 将下一个元素放入优先级队列中
        auto isst = heap.top().isst;
        if(!isst->default_iterator.isEnd()){
            heap.push(priority_queue_type(*(isst->default_iterator), isst, heap.top().level, heap.top().id));
            ++isst->default_iterator;
        }
    }

    // 添加删除本层和上层sst的Record到manifest文件，添加新的Record到manifest文件，并获得文件id
    std::vector<uint64_t> sst_ids;
    sst_ids.reserve(new_ssts.size());
    for(auto sst:ssts){
        _manifest.DeleteSSTRecord(sst->GetID());
    }
    for(auto sst:last_level_ssts){
        _manifest.DeleteSSTRecord(sst->GetID());
    }
    for(auto sst:new_ssts){
        sst_ids.push_back(_manifest.CreateSSTRecord(_level,sst->GetMinKey(), sst->GetMaxKey()));
    }

    std::unique_lock Lock1(level_mutex);
    std::unique_lock Lock2(last_level.level_mutex);
    // 删除本Level和上个Level的SSTable
    Clear();
    last_level.Clear();
    // 添加新的SSTable
    for(int i=0; i<new_ssts.size();i++){
        new_ssts[i]->Rename(sst_ids[i], option.DB_PATH + std::to_string(sst_ids[i]) + ".sst");
        ssts.push_back(new_ssts[i]);
    }
}
