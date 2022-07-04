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

    void CreateSSTFile();
    void WriteDataBlock(uint64_t key, const std::string& value);
    void WriteMetaData();
    size_t WritenSize(){return pos_db+offset_db+offset_ib;} // 已写入文件大小（包括缓冲区）

    void Rename(uint64_t newid, std::string&& newpath);

    static void LoadIndexBlockFromBuf(char* buf, size_t bufsz, std::map<uint64_t,std::pair<size_t,size_t>>& ib);
    static void LoadDataBlockFromBuf(char* buf, size_t bufsz, std::map<uint64_t,std::string>& db);

    uint64_t GetMinKey(){return min_key;}
    uint64_t GetMaxKey(){return max_key;}
    const std::string& GetPath() const{return path;}
    uint64_t GetID(){return tbl_id;}

    size_t GetIndexBlockPosition() const{return ib_pos;}
    size_t GetIndexBlockSize() const{return ib_sz;}

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

    // SST文件写入相关的临时变量
    std::ofstream* writer;
    char* buf_db; // DataBlock的写缓冲区
    char* buf_ib; // IndexBlock的写缓冲区
    size_t buf_ib_sz; // IndexBlock写缓冲区的大小
    int offset_db = 0; // buf_db的offset
    int offset_ib = 0; // buf_ib的offset
    int max_db_key; // IndexBlock的索引
    size_t pos_db = 0; // DataBlock在文件中的起始地址
    bool first_write = true; // 是否是第一次写入

};

// 使用迭代器遍历SSTable，用于MajorCompaction
class IterableSSTable{
public:
    IterableSSTable(const SSTable& sst, const Option& option);
    ~IterableSSTable();

    struct Iterator{
    public:
        Iterator(){}
        Iterator(IterableSSTable* it,size_t pos);
        void init(IterableSSTable* it,size_t pos);

        bool operator!=(const Iterator& it) const { return
            (file_pos!=it.file_pos) && (file_pos<_it->ib_pos)&&(it.file_pos<_it->ib_pos); }
        Iterator& operator++();
        std::pair<uint64_t, std::string> operator*() const;

        bool isEnd(){return file_pos >= _it->ib_pos;}
    private:
        size_t file_pos{}; // 迭代器在文件中的位置
        size_t offset{}; // 迭代器在缓冲区中的位置
        IterableSSTable* _it{};
    };
    Iterator begin() { return Iterator(this, 0); }
    Iterator end() { return Iterator(this, ib_pos); }

    Iterator default_iterator;

private:
    std::ifstream* reader;
    size_t ib_pos; // IndexBlock的起始位置

    size_t MAX_BUF_SZ;
    char* buf;
    size_t buf_pos; // 缓冲区的开始在文件中的位置
    size_t buf_sz;
};

#endif //LSMTREE_SSTABLE_H
