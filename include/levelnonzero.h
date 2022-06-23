#ifndef LSMTREE_LEVELNONZERO_H
#define LSMTREE_LEVELNONZERO_H

#include "levelzero.h"

class LevelNonZero{
public:
    LevelNonZero(Option& op, Manifest& manifest, Cache* tablecache, Cache* blockcache);
    LevelNonZero(Option& op, Manifest& manifest, Utils::LevelMetaDataType& levelmetadata, Cache* tablecache, Cache* blockcache);

    ~LevelNonZero();

    std::string Get(uint64_t key, bool* is_failed) const;

    mutable std::shared_mutex level_mutex;
private:
    std::vector<SSTable*> ssts;

    Cache* tbl_cache; // TableCache
    Cache* blk_cache; // BlockCache

    Option& option;
    Manifest& _manifest;
};

#endif //LSMTREE_LEVELNONZERO_H
