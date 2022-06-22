#include "levelzero.h"

LevelZero::LevelZero(Option& op,Cache* tablecache, Cache* blockcache): tbl_cache(tablecache), blk_cache(blockcache){
    option = op;
}

std::string LevelZero::Get(uint64_t key, bool *is_failed) const {

}

void LevelZero::MinorCompaction(const SkipList &sl) {

}
