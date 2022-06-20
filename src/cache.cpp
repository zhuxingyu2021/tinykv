# include "cache.h"
# include "sstable.h"

void TableCacheMem::LoadBuf(char *buf, size_t bufsz) {
    SSTable::LoadIndexBlockFromBuf(buf,bufsz,ib);
}

void BlockCacheMem::LoadBuf(char *buf, size_t bufsz) {
    SSTable::LoadDataBlockFromBuf(buf,bufsz,db);
}

// 查找key对应的缓存，如果找到，返回缓存地址，否则返回nullptr
void* Cache::Get(uint64_t key, uint64_t key2){
// TODO
    return nullptr;
}

// 把pointer指向的内存单元标记为缓存
void Cache::Put(uint64_t key, uint64_t key2, void* pointer, size_t size){
// TODO
}

// 从缓存中删除指定项
void Cache::Evict(uint64_t key, uint64_t key2) {
// TODO
}
