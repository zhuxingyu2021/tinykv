#ifndef LSMTREE_LEVEL_H
#define LSMTREE_LEVEL_H

#include <mutex>
#include <shared_mutex>
#include "sstable.h"
#include <vector>
#include <string>

#include <iostream>

class Level{
public:
    virtual std::string Get(uint64_t key, bool* is_failed) const {std::cerr << "Error!" << std::endl;
        exit(-1);}
    virtual int GetLevel() const{std::cerr << "Error!" << std::endl;
        exit(-1);}
    virtual const std::vector<SSTable*>& GetSSTables() const {std::cerr << "Error!" << std::endl;
        exit(-1);}
    virtual void Clear() {std::cerr << "Error!" << std::endl;
        exit(-1);}

    mutable std::shared_mutex level_mutex;
};

#endif //LSMTREE_LEVEL_H
