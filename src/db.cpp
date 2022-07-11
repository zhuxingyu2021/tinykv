#include "db.h"
#include "utils.h"
#include <cassert>
#include <filesystem>

// 初始化
void DB::initdb(){
    // 创建MemTable
    mem = new SkipList;

    // 初始化TableCache和BlockCache
    tbl_cache= nullptr; blk_cache= nullptr;
    if(option.TABLECHCHE_ENABLED){
        tbl_cache=new Cache(option, Cache::CACHE_TYPE_TABLECACHE);
    }
    if(option.BLOCKCACHE_ENABLED){
        blk_cache=new Cache(option, Cache::CACHE_TYPE_BLOCKCACHE);
    }

    // 读取并重写Manifest文件
    std::filesystem::create_directory(option.DB_PATH);
    manifest = new Manifest(option);
    const Manifest::RecordType& record = manifest->GetRecord();

    if(!record.empty()){
        // 构建各个Level
        std::vector<Utils::LevelMetaDataType> level_metadatas(option.MAX_LEVEL, Utils::LevelMetaDataType());
        for(auto entry:record){
            auto level = entry.second.level;
            level_metadatas[level][entry.first] = std::make_pair(entry.second.min_key, entry.second.max_key);
        }

        auto level=0;
        for(auto iter = level_metadatas.begin();iter!=level_metadatas.end();iter++, level++){
            auto& lvlmeta = *iter;
            if(!lvlmeta.empty()) levels.push_back(new Level(option, *manifest, level, lvlmeta, tbl_cache, blk_cache));
            else levels.push_back(new Level(option, *manifest, level, tbl_cache, blk_cache));
        }
    }
}

DB::DB():compaction_thread(nullptr), compaction_thread_scheduled(false) {
    imm_mem.sl = nullptr;
    initdb();
}

DB::DB(Option& op):compaction_thread(nullptr), compaction_thread_scheduled(false) {
    imm_mem.sl = nullptr;
    option = op;
    initdb();
}

DB::~DB() {
    while(compaction_thread_scheduled);
    assert(imm_mem.sl==nullptr);

    option.MAJOR_COMPACTION_ENABLED= false;

    // 将MemTable数据写入磁盘
    if(mem->Size()>0){
        imm_mem.sl = mem;

        compaction_thread_scheduled = true;
        delete compaction_thread;
        compaction_thread = new std::thread(&DB::compaction, this);
        compaction_thread->join();
        delete compaction_thread;
    }
    assert(imm_mem.sl==nullptr);

    for(auto level:levels) delete level;

    delete tbl_cache; delete blk_cache;
    delete manifest;
}

// 获取键key对应的值，若不存在，则返回空字符串
std::string DB::Get(uint64_t key) const {
    bool find_failed;
    // 1. 在MemTable中查找
    std::string val=mem->Get(key,&find_failed);

    if(find_failed){
        // 2. 在Immutable MemTable中查找
        std::shared_lock Lock(imm_mem.mutex);
        if(imm_mem.sl){
            val = imm_mem.sl->Get(key,&find_failed);
        }
        Lock.unlock();

        if(find_failed){
            // 3. 在各个Level中查找
            for(auto level:levels){
                val = level->Get(key,&find_failed);
                if(!find_failed) break;
            }
        }
    }
    return val;
}

// 将(key, val)插入或更新到lsm tree
void DB::Put(uint64_t key, const std::string& val) {
    mem->Put(key, val);
    schedule();
}

// 删除键key
void DB::Del(uint64_t key) {
    mem->Put(key, "");
    schedule();
}

// 调度后台compaction线程
void DB::schedule() {
    if(!compaction_thread_scheduled){
        if(mem->Space()>=option.MAX_MEMTABLE_SIZE) { // 若MemTable超过一定大小，就调度compaction线程执行minor compaction
            assert(imm_mem.sl==nullptr);
            // 将MemTable转变为Immutable MemTable
            imm_mem.sl = mem;
            mem = new SkipList;

            compaction_thread_scheduled = true;
            delete compaction_thread;
            compaction_thread = new std::thread(&DB::compaction, this);

            compaction_thread->detach();
        }
    }
}

//后台compaction线程做的事情
void DB::compaction() {
    compaction_thread_scheduled = true;
    if(imm_mem.sl){
        if(levels.empty()) levels.push_back(new Level(option, *manifest, 0, tbl_cache, blk_cache));
        levels[0]->MinorCompaction(imm_mem);
    }
    if(option.MAJOR_COMPACTION_ENABLED){
        for(int i=0;i<levels.size()&&i<option.MAX_LEVEL-1;i++){
            if((i==0 && levels[i]->GetSSTables().size()>=option.MAX_LEVEL_0_FILES) || (levels[i]->Space()>=((option.MAX_LEVEL_0_SIZE)<<i))){
                // Level i 占用空间过大 或 Level 0文件数量超过上限
                if(i+1>=levels.size()) {
                    levels.push_back(new Level(option, *manifest, i+1, tbl_cache, blk_cache));
                }
                levels[i+1]->MajorCompaction((Level*)levels[i]);
            }
        }
    }
    compaction_thread_scheduled = false;
}
