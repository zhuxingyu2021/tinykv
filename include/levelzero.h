#ifndef LSMTREE_LEVELZERO_H
#define LSMTREE_LEVELZERO_H

#include <mutex>
#include <shared_mutex>
#include "sstable.h"
#include "skiplist.h"
#include <vector>

#include "option.h"
#include "utils.h"
#include "manifest.h"

class LevelZero{
public:
    LevelZero(Option& op, Manifest& manifest, Cache* tablecache, Cache* blockcache);
    LevelZero(Option& op, Manifest& manifest, Utils::LevelMetaDataType& levelmetadata, Cache* tablecache, Cache* blockcache);

    ~LevelZero();

    std::string Get(uint64_t key, bool* is_failed) const;

    void MinorCompaction(Utils::ImmutableMemTable& imm_mem);

    mutable std::shared_mutex level_0_mutex;
private:
    std::vector<SSTable*> ssts;

    Cache* tbl_cache; // TableCache
    Cache* blk_cache; // BlockCache

    Option& option;
    Manifest& _manifest;
};

#endif //LSMTREE_LEVELZERO_H
