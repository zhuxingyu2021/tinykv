# include "cache.h"
# include "sstable.h"

TableCacheMem::TableCacheMem(uint64_t key, char *buf, size_t bufsz): CacheMem(key,0) {
    SSTable::LoadIndexBlockFromBuf(buf,bufsz,ib);
}

BlockCacheMem::BlockCacheMem(uint64_t key, uint64_t key2, char *buf, size_t bufsz): CacheMem(key, key2){
    SSTable::LoadDataBlockFromBuf(buf,bufsz,db);
}

Cache::Cache(Option& op,uint8_t cachetype):cache_type(cachetype), total_size(0) {
    option = op;
    if(cachetype==CACHE_TYPE_BLOCKCACHE) max_size=option.BLOCKCACHE_SIZE;
    else if(cachetype==CACHE_TYPE_TABLECACHE) max_size=option.TABLECACHE_SIZE;
    else return;
}

Cache::~Cache(){
    for(auto item:LRUList){
        if(cache_type==CACHE_TYPE_BLOCKCACHE) delete (BlockCacheMem*)item;
        else if(cache_type==CACHE_TYPE_TABLECACHE) delete (TableCacheMem*)item;
        else return;
    }
}

// 查找key对应的缓存，如果找到，返回缓存地址，否则返回nullptr
CacheMem* Cache::Get(uint64_t key, uint64_t key2){
    auto iter = LRUList.begin();
    for(;iter!=LRUList.end();iter++){
        if(((*iter)->_key==key) && ((*iter)->_key2==key2)){
            break;
        }
    }
    if(iter==LRUList.end()){ // 未找到
        return nullptr;
    }

    // 若找到指定的缓存，根据LRU算法，需要把缓存插入到链表表头
    auto mem = *iter;
    LRUList.erase(iter);
    LRUList.insert(LRUList.begin(), mem);
    return mem;
}

// 把pointer指向的内存单元标记为缓存，并返回缓存地址
CacheMem* Cache::Put(uint64_t key, uint64_t key2, char* pointer, size_t size){
    if(size>max_size) throw CacheSizeSmallException();

    CacheMem* cm = nullptr;
    if(cache_type==CACHE_TYPE_BLOCKCACHE) cm = new BlockCacheMem(key, key2, pointer,size);
    else if(cache_type==CACHE_TYPE_TABLECACHE) cm = new TableCacheMem(key, pointer,size);
    else return nullptr;
    cm->sz = size;

    // LRU淘汰链表最后的元素
    while(total_size+cm->sz>max_size){
        auto last = LRUList.end();
        last--;
        total_size -= (*last)->sz;
        if(cache_type==CACHE_TYPE_BLOCKCACHE) delete (BlockCacheMem*)(*last);
        else if(cache_type==CACHE_TYPE_TABLECACHE) delete (TableCacheMem*)(*last);
        LRUList.erase(last);
    }

    // 把当前元素插入到链表表头
    total_size += cm->sz;
    LRUList.insert(LRUList.begin(), cm);
    return cm;
}

// 从缓存中删除指定项
void Cache::Evict(uint64_t key, uint64_t key2) {
    // 查找指定项
    auto iter = LRUList.begin();
    for(;iter!=LRUList.end();iter++){
        if(((*iter)->_key==key) && ((*iter)->_key2==key2)){
            break;
        }
    }
    if(iter==LRUList.end()) return;

    // 删除指定项
    total_size -= (*iter)->sz;
    if(cache_type==CACHE_TYPE_BLOCKCACHE) delete (BlockCacheMem*)(*iter);
    else if(cache_type==CACHE_TYPE_TABLECACHE) delete (TableCacheMem*)(*iter);
    LRUList.erase(iter);
}
