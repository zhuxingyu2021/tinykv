#ifndef LSMTREE_SSTABLE_H
#define LSMTREE_SSTABLE_H

#include <cstdint>
#include <string>
#include "skiplist.h"

class SSTable{
public:
    SSTable(std::string dbpath);

    void BuildFromMem(SkipList& sl);
private:
    std::string path; //SSTable存放路径
};

#endif //LSMTREE_SSTABLE_H
