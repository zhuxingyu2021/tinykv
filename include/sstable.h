#ifndef LSMTREE_SSTABLE_H
#define LSMTREE_SSTABLE_H

#include <cstdint>
#include <string>
#include <map>
#include "cache.h"
#include "skiplist.h"

class SSTable{
public:
    SSTable(uint64_t id, Cache* tablecache, Cache* blockcache);

    void BuildFromMem(SkipList& sl);

    static void LoadIndexBlockFromBuf(char* buf, size_t bufsz, std::map<uint64_t,std::pair<size_t,size_t>>& ib);
    static void LoadDataBlockFromBuf(char* buf, size_t bufsz, std::map<uint64_t,std::string>& db);
private:
    uint64_t tbl_id; //文件id
    std::string path; //SSTable存放路径
    size_t ib_pos; //IndexBlock在文件中的位置
    Cache* tbl_cache;
    Cache* blk_cache;

    bool loaded; //是否创建/加载过该SSTable
};

#endif //LSMTREE_SSTABLE_H
