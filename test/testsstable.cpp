#include "sstable.h"
#include "skiplist.h"
#include <iostream>
#include <map>
#include "option.h"
#include <cassert>

#define INSERT_COUNT 1000000
#define RANGE_MAX 100000000

#define ASSERT_RELEASE(expr) \
if(!(expr)){std::cout << "Assert Failed! In source file"<< __FILE__ << ",line " << __LINE__ << std::endl;exit(-1);}

int main()
{
    SkipList sl;
    std::map<uint64_t,std::string > m;
    Option default_option;

    for(int i=0;i<INSERT_COUNT;i++){
        uint64_t key = rand()%RANGE_MAX;
        std::string val = std::string("Element:")+std::to_string(key)+std::string(" ")+std::to_string(rand())+std::string(" end");
        sl.Put(key, val);

        m[key] = val;
    }
    Cache *tblcache= nullptr, *blkcache= nullptr;
    if(default_option.TABLECHCHE_ENABLED){
        tblcache=new Cache(default_option, Cache::CACHE_TYPE_TABLECACHE);
    }
    if(default_option.BLOCKCACHE_ENABLED){
        blkcache=new Cache(default_option, Cache::CACHE_TYPE_BLOCKCACHE);
    }
    SSTable sst(default_option, 0,"test.sst", tblcache, blkcache);
    sst.BuildFromMem(sl);

    for(auto kv: m){
        ASSERT_RELEASE(sst.Get(kv.first, nullptr)==kv.second);
    }

    delete tblcache;
    delete blkcache;

}
