#include "sstable.h"
#include "skiplist.h"

#define INSERT_COUNT 50
#define RANGE_MAX 10000

int main()
{
    SkipList sl;
    for(int i=0;i<INSERT_COUNT;i++){
        auto key = rand()%RANGE_MAX;
        std::string val = std::string("Element:")+std::to_string(key)+std::string(" ")+std::to_string(rand())+std::string(" end");
        sl.Put(key, val);
    }
    SSTable sst(0, nullptr, nullptr);
    sst.BuildFromMem(sl);
}
