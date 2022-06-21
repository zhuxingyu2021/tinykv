#include "sstable.h"
#include "skiplist.h"
#include <iostream>
#include <map>
#include "option.h"
#include <cassert>

#define INSERT_COUNT 500000
#define RANGE_MAX 40000

#define ASSERT_RELEASE(expr) \
if(!(expr)){std::cout << "Assert Failed! In source file"<< __FILE__ << ",line " << __LINE__ << std::endl;exit(-1);}

int main()
{
    SkipList sl;
    std::map<uint64_t,std::string > m;
    for(int i=0;i<INSERT_COUNT;i++){
        uint64_t key = rand()%RANGE_MAX;
        std::string val = std::string("Element:")+std::to_string(key)+std::string(" ")+std::to_string(rand())+std::string(" end");
        sl.Put(key, val);

        m[key] = val;
    }
    Cache *tblcache= nullptr, *blkcache= nullptr;
    if(Option::TABLECHCHE_ENABLED){
        tblcache=new Cache(Cache::CACHE_TYPE_TABLECACHE);
    }
    if(Option::BLOCKCACHE_ENABLED){
        blkcache=new Cache(Cache::CACHE_TYPE_BLOCKCACHE);
    }
    SSTable sst(0, tblcache, blkcache);
    sst.BuildFromMem(sl);

    for(auto kv: m){
        ASSERT_RELEASE(sst.Get(kv.first, nullptr)==kv.second);
    }

    if(Option::TABLECHCHE_ENABLED){
        delete tblcache;
    }
    if(Option::BLOCKCACHE_ENABLED){
        delete blkcache;
    }

}
