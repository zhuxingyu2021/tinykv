#ifndef LSMTREE_DB_H
#define LSMTREE_DB_H

#include <mutex>
#include <thread>
#include <cstdint>
#include "skiplist.h"
#include "levelzero.h"
#include "cache.h"
#include "option.h"

class DB{
public:
    explicit DB();
    explicit DB(Option& op);
    ~DB();

    std::string Get(uint64_t key) const;
    void Put(uint64_t key, const std::string& val);
    void Del(uint64_t key);

private:
    SkipList* mem; // MemTable
    SkipList* imm_mem; // Immutable MemTable
    mutable std::mutex imm_mem_mutex; // Immutable MemTable的mutex

    std::thread* compaction_thread; // 后台compaction线程
    bool compaction_thread_scheduled; // 判断后台compaction线程是否被调度

    void initdb();

    void schedule();
    void compaction();

    LevelZero* level_0;

    Cache* tbl_cache; // TableCache
    Cache* blk_cache; // BlockCache

    Option option;
};

#endif //LSMTREE_DB_H
