#ifndef LSMTREE_DB_H
#define LSMTREE_DB_H

#include <mutex>
#include <shared_mutex>
#include <thread>
#include <cstdint>
#include "skiplist.h"
#include "level.h"
#include "cache.h"
#include "option.h"
#include "manifest.h"

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

    Utils::ImmutableMemTable imm_mem; // Immutable MemTable

    std::thread* compaction_thread; // 后台compaction线程
    volatile bool compaction_thread_scheduled; // 判断后台compaction线程是否被调度

    void initdb();

    void schedule();
    void compaction();

    std::vector<Level*> levels;

    Cache* tbl_cache; // TableCache
    Cache* blk_cache; // BlockCache

    Option option;
    Manifest* manifest;
};

#endif //LSMTREE_DB_H
