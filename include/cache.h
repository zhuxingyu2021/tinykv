#ifndef LSMTREE_CACHE_H
#define LSMTREE_CACHE_H

#include <cstdint>
#include <cstddef>
#include <list>
#include <string>

struct CachedMem{
    void* pointer; // 缓存的地址
    size_t size; // 缓存大小
};

// 采用LRU算法实现TableCache和BlockCache
class Cache{
public:
    explicit Cache() {}
    void* Get(uint64_t key);
    void Put(uint64_t key, void* pointer, size_t size);
    void Evict(uint64_t key);
private:
    std::list<CachedMem> LRUList;
    size_t total_size; // 总缓存大小
};

#endif //LSMTREE_CACHE_H
