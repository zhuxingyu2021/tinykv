#include "sstable.h"
#include "skiplist.h"

#define INSERT_COUNT 1000000
#define RANGE_MAX 10000

int main()
{
    SkipList sl;
    for(int i=0;i<INSERT_COUNT;i++){
        auto key = rand()%RANGE_MAX;
        std::string val = std::to_string(key)+std::to_string(rand());
        sl.Put(key, val);
    }
    SSTable sst("testsst.db");
    sst.BuildFromMem(sl);
}
