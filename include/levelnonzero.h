#ifndef LSMTREE_LEVELNONZERO_H
#define LSMTREE_LEVELNONZERO_H

#include "levelzero.h"

class LevelNonZero{
public:
    LevelNonZero(Option& op, Manifest& manifest, int level, Cache* tablecache, Cache* blockcache);
    LevelNonZero(Option& op, Manifest& manifest, int level, Utils::LevelMetaDataType& levelmetadata, Cache* tablecache, Cache* blockcache);

    ~LevelNonZero();

    std::string Get(uint64_t key, bool* is_failed) const;

    const std::vector<SSTable*>& GetSSTables() const;

    void MajorCompaction(LevelZero& last_level);
    void MajorCompaction(LevelNonZero& last_level);

    mutable std::shared_mutex level_mutex;
private:
    std::vector<SSTable*> ssts;
    bool empty;

    Cache* tbl_cache; // TableCache
    Cache* blk_cache; // BlockCache

    Option& option;
    Manifest& _manifest;

    int _level;
};

#endif //LSMTREE_LEVELNONZERO_H
