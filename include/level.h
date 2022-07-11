#ifndef LSMTREE_LEVEL_H
#define LSMTREE_LEVEL_H

#include <mutex>
#include <shared_mutex>
#include "sstable.h"
#include <vector>
#include <string>
#include <iostream>
#include "skiplist.h"

#include "option.h"
#include "utils.h"
#include "manifest.h"

class Level{
public:
    Level(Option& op, Manifest& manifest, int level, Cache* tablecache, Cache* blockcache);
    Level(Option& op, Manifest& manifest, int level, Utils::LevelMetaDataType& levelmetadata, Cache* tablecache, Cache* blockcache);

    ~Level();

    std::string Get(uint64_t key, bool* is_failed) const;
    int GetLevel() const{return _level;}

    const std::vector<SSTable*>& GetSSTables() const;

    void Clear();

    void MajorCompaction(Level* last_level);
    void MinorCompaction(Utils::ImmutableMemTable& imm_mem);

    size_t Space(){
        size_t size = 0;
        for(auto sst:ssts){
            size += sst->GetFileSize();
        }
        return size;
    }

    mutable std::shared_mutex level_mutex;

private:

    std::vector<SSTable*> ssts;

    Cache* tbl_cache; // TableCache
    Cache* blk_cache; // BlockCache

    Option& option;
    Manifest& _manifest;

    int _level;
};

#endif //LSMTREE_LEVEL_H
