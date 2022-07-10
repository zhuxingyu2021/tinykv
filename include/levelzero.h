#ifndef LSMTREE_LEVELZERO_H
#define LSMTREE_LEVELZERO_H

#include "level.h"
#include "skiplist.h"

#include "option.h"
#include "utils.h"
#include "manifest.h"

class LevelZero:public Level{
public:
    LevelZero(Option& op, Manifest& manifest, Cache* tablecache, Cache* blockcache);
    LevelZero(Option& op, Manifest& manifest, Utils::LevelMetaDataType& levelmetadata, Cache* tablecache, Cache* blockcache);

    ~LevelZero();

    virtual std::string Get(uint64_t key, bool* is_failed) const;
    virtual int GetLevel() const{return 0;}

    virtual const std::vector<SSTable*>& GetSSTables() const;
    virtual void Clear();

    size_t Space(){
        size_t size = 0;
        for(auto sst:ssts){
            size += sst->GetFileSize();
        }
        return size;
    }

    void MinorCompaction(Utils::ImmutableMemTable& imm_mem);

private:
    std::vector<SSTable*> ssts;

    Cache* tbl_cache; // TableCache
    Cache* blk_cache; // BlockCache

    Option& option;
    Manifest& _manifest;
};

#endif //LSMTREE_LEVELZERO_H
