#ifndef LSMTREE_SSTABLE_H
#define LSMTREE_SSTABLE_H

#include <cstdint>
#include <string>
#include <map>
#include <fstream>
#include "cache.h"
#include "skiplist.h"
#include "option.h"

class SSTable{
public:
    SSTable(Option& op, std::string&& path, Cache* tablecache, Cache* blockcache);
    SSTable(Option& op,uint64_t id,std::string&& path, Cache* tablecache, Cache* blockcache, uint64_t minkey, uint64_t maxkey);

    std::string Get(uint64_t key, bool* is_failed) const;

    void BuildFromMem(SkipList& sl);

    void Rename(uint64_t newid, std::string&& newpath);

    static void LoadIndexBlockFromBuf(char* buf, size_t bufsz, std::map<uint64_t,std::pair<size_t,size_t>>& ib);
    static void LoadDataBlockFromBuf(char* buf, size_t bufsz, std::map<uint64_t,std::string>& db);

    uint64_t GetMinKey(){return min_key;}
    uint64_t GetMaxKey(){return max_key;}

    // footer区域的大小
    static const long FOOTER_SIZE=2*sizeof(size_t);
private:
    uint64_t min_key; //最小key
    uint64_t max_key; //最大key

    uint64_t tbl_id; //文件id
    std::string path; //SSTable存放路径
    size_t ib_pos; //IndexBlock在文件中的位置
    size_t ib_sz; //IndexBlock大小
    Cache* tbl_cache;
    Cache* blk_cache;

    Option& option;
};

#endif //LSMTREE_SSTABLE_H
