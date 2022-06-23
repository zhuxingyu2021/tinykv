# include "levelnonzero.h"

LevelNonZero::LevelNonZero(Option &op, Manifest &manifest, int level, Cache *tablecache, Cache *blockcache): option(op),
    _manifest(manifest), tbl_cache(tablecache), blk_cache(blockcache), _level(level), empty(true){}

LevelNonZero::LevelNonZero(Option &op, Manifest &manifest, int level, Utils::LevelMetaDataType &levelmetadata, Cache *tablecache,
                           Cache *blockcache): option(op), _manifest(manifest), tbl_cache(tablecache),blk_cache(blockcache),
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

// MajorCompaction
void LevelNonZero::MajorCompaction(LevelZero &last_level) {
    const std::vector<SSTable*>& last_level_ssts = last_level.GetSSTables();

}

// MajorCompaction
void LevelNonZero::MajorCompaction(LevelNonZero &last_level) {
    const std::vector<SSTable*>& last_level_ssts = last_level.GetSSTables();
}
