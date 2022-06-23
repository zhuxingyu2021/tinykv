#ifndef LSMTREE_UTILS_H
#define LSMTREE_UTILS_H

#include <vector>
#include <map>
#include <mutex>
#include "skiplist.h"
#include <shared_mutex>

namespace Utils{
    // SSTid -> (min_key, max_key)
    typedef std::map<uint64_t, std::pair<uint64_t,uint64_t>> LevelMetaDataType;

    struct ImmutableMemTable{
        SkipList* sl;
        mutable std::shared_mutex mutex;
    };
}

#endif //LSMTREE_UTILS_H
