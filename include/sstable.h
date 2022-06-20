#ifndef LSMTREE_SSTABLE_H
#define LSMTREE_SSTABLE_H

#include <cstdint>
#include <string>
#include "cache.h"
#include "skiplist.h"

class SSTable{
public:
    SSTable(std::string dbpath, Cache* tablecache, Cache* blockcache):
        path(dbpath),table_cache(tablecache),block_cache(blockcache){}

    void BuildFromMem(SkipList& sl);
private:
    std::string path; //SSTable存放路径
    Cache* table_cache;
    Cache* block_cache;
};

#endif //LSMTREE_SSTABLE_H
