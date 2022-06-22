#ifndef LSMTREE_LEVELZERO_H
#define LSMTREE_LEVELZERO_H

#include <mutex>
#include "sstable.h"
#include "skiplist.h"
#include <vector>

class LevelZero{
public:
    LevelZero(Cache* tablecache, Cache* blockcache);
    std::string Get(uint64_t key, bool* is_failed) const;

    void MinorCompaction(const SkipList& sl);

    mutable std::mutex level_0_mutex;
private:
    std::vector<SSTable*> ssts;

    Cache* tbl_cache; // TableCache
    Cache* blk_cache; // BlockCache
};

#endif //LSMTREE_LEVELZERO_H
