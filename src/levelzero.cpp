#include "levelzero.h"
#include "option.h"

LevelZero::LevelZero(Cache* tablecache, Cache* blockcache): tbl_cache(tablecache), blk_cache(blockcache){

}

std::string LevelZero::Get(uint64_t key, bool *is_failed) const {

}

void LevelZero::MinorCompaction(const SkipList &sl) {

}
