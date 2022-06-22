#ifndef LSMTREE_UTILS_H
#define LSMTREE_UTILS_H

#include <vector>
#include <map>
#include <mutex>
#include "skiplist.h"

namespace Utils{
    // SSTid -> (min_key, max_key)
    typedef std::map<uint64_t, std::pair<uint64_t,uint64_t>> LevelMetaDataType;

    struct ImmutableMemTable{
        SkipList* sl;
        mutable std::mutex mutex;
    };
}

#endif //LSMTREE_UTILS_H
