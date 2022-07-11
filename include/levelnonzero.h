#ifndef LSMTREE_LEVELNONZERO_H
#define LSMTREE_LEVELNONZERO_H

#include "level.h"
#include "skiplist.h"

#include "option.h"
#include "utils.h"
#include "manifest.h"

class LevelNonZero:public Level{
public:
    LevelNonZero(Option& op, Manifest& manifest, int level, Cache* tablecache, Cache* blockcache);
    LevelNonZero(Option& op, Manifest& manifest, int level, Utils::LevelMetaDataType& levelmetadata, Cache* tablecache, Cache* blockcache);

    virtual ~LevelNonZero();

    virtual std::string Get(uint64_t key, bool* is_failed) const;
    virtual int GetLevel() const{return _level;}

    virtual const std::vector<SSTable*>& GetSSTables() const;

    virtual void Clear();

    void MajorCompaction(Level* last_level);

    size_t Space(){
        size_t size = 0;
        for(auto sst:ssts){
            size += sst->GetFileSize();
        }
        return size;
    }

private:
    std::vector<SSTable*> ssts;

    Cache* tbl_cache; // TableCache
    Cache* blk_cache; // BlockCache

    Option& option;
    Manifest& _manifest;

    int _level;
};

#endif //LSMTREE_LEVELNONZERO_H
