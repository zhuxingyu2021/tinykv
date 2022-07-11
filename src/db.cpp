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

        level_0 = new LevelZero(option, *manifest, level_metadatas[0], tbl_cache, blk_cache);

        auto iter = level_metadatas.begin();
        auto level = 1;
        iter++;
        for(;iter!=level_metadatas.end();iter++, level++){
            auto& lvlmeta = *iter;
            if(!lvlmeta.empty()) level_n0.push_back(new LevelNonZero(option, *manifest, level, lvlmeta, tbl_cache, blk_cache));
            else level_n0.push_back(new LevelNonZero(option, *manifest, level, tbl_cache, blk_cache));
        }
    }else {
        level_0 = new LevelZero(option, *manifest, tbl_cache, blk_cache);
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

    delete level_0;
    for(auto level:level_n0) delete level;

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
            // 3. 在Level 0中查找
            val = level_0->Get(key,&find_failed);
            if(find_failed){
                // 4. 在各个Level中查找
                for(auto level:level_n0){
                    val = level->Get(key,&find_failed);
                    if(!find_failed) break;
                }
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
        level_0->MinorCompaction(imm_mem);
    }
    if(option.MAJOR_COMPACTION_ENABLED){
        if(level_0->Space()>=option.MAX_LEVEL_0_SIZE || level_0->GetSSTables().size()>=option.MAX_LEVEL_0_FILES){
            if(level_n0.empty()){
                level_n0.push_back(new LevelNonZero(option, *manifest, 1, tbl_cache, blk_cache));
            }
            level_n0[0]->MajorCompaction((Level*)level_0);
        }
        for(int i=0;i<level_n0.size()&&i<option.MAX_LEVEL-1;i++){
            if(level_n0[i]->Space()>=((option.MAX_LEVEL_0_SIZE)<<(i+1))){
                // Level i+1 占用空间过大
                if(i+1>=level_n0.size()) {
                    level_n0.push_back(new LevelNonZero(option, *manifest, i+2, tbl_cache, blk_cache));
                }
                level_n0[i+1]->MajorCompaction((Level*)level_n0[i]);
            }
        }
    }
    compaction_thread_scheduled = false;
}
