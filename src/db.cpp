#include "db.h"
#include "option.h"
#include <cassert>

DB::DB():compaction_thread(nullptr), compaction_thread_scheduled(false), imm_mem(nullptr) {
    mem = new SkipList;

    tbl_cache= nullptr; blk_cache= nullptr;
    if(Option::TABLECHCHE_ENABLED){
        tbl_cache=new Cache(Cache::CACHE_TYPE_TABLECACHE);
    }
    if(Option::BLOCKCACHE_ENABLED){
        blk_cache=new Cache(Cache::CACHE_TYPE_BLOCKCACHE);
    }

    level_0 = new LevelZero(tbl_cache, blk_cache);
}

DB::~DB() {
    if(compaction_thread){
        if(compaction_thread->joinable()) compaction_thread->join();
    }
    assert(imm_mem==nullptr);

    // 将MemTable数据写入磁盘
    if(mem->Size()>0){
        imm_mem = mem;

        compaction_thread_scheduled = true;
        delete compaction_thread;
        compaction_thread = new std::thread(&DB::compaction, this);
        compaction_thread->join();
        delete compaction_thread;
    }
    assert(imm_mem==nullptr);

    delete level_0;

    delete tbl_cache; delete blk_cache;
}

// 获取键key对应的值，若不存在，则返回空字符串
std::string DB::Get(uint64_t key) const {
    bool find_failed;
    // 1. 在MemTable中查找
    std::string val=mem->Get(key,&find_failed);

    if(find_failed){
        // 2. 在Immutable MemTable中查找
        imm_mem_mutex.lock();
        if(imm_mem){
            val = imm_mem->Get(key,&find_failed);
        }
        imm_mem_mutex.unlock();

        if(find_failed){
            // 3. 在Level 0中查找
            //TODO
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
        if(mem->Space()>=Option::MAX_MEMTABLE_SIZE) { // 若MemTable超过一定大小，就调度compaction线程执行minor compaction
            assert(imm_mem==nullptr);
            // 将MemTable转变为Immutable MemTable
            imm_mem = mem;
            mem = new SkipList;

            compaction_thread_scheduled = true;
            delete compaction_thread;
            compaction_thread = new std::thread(&DB::compaction, this);
        }
    }
}

//后台compaction线程做的事情
void DB::compaction() {
    // TODO
    imm_mem_mutex.lock();
    delete imm_mem;
    imm_mem_mutex.unlock();
    compaction_thread_scheduled = true;
}
