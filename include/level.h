#ifndef LSMTREE_LEVEL_H
#define LSMTREE_LEVEL_H

#include <mutex>
#include <shared_mutex>
#include "sstable.h"
#include <vector>
#include <string>

class Level{
public:
    virtual std::string Get(uint64_t key, bool* is_failed) const {}
    virtual int GetLevel() {}
    virtual const std::vector<SSTable*>& GetSSTables() const {}
    virtual void Clear() {}

    mutable std::shared_mutex level_mutex;
};

#endif //LSMTREE_LEVEL_H
