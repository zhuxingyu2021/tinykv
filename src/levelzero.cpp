#include "levelzero.h"

LevelZero::LevelZero(Option& op, Manifest& manifest,Cache* tablecache, Cache* blockcache): tbl_cache(tablecache),
blk_cache(blockcache), option(op), _manifest(manifest){}

LevelZero::LevelZero(Option& op, Manifest& manifest, Utils::LevelMetaDataType& levelmetadata, Cache* tablecache, Cache* blockcache):
tbl_cache(tablecache), blk_cache(blockcache), option(op), _manifest(manifest){
    for(auto sst: levelmetadata){
        auto id = sst.first;
        ssts.push_back(new SSTable(op, id, std::string(op.DB_PATH) + std::to_string(id) + std::string(".sst"),
                                   tablecache, blockcache, sst.second.first, sst.second.second));
    }
}

LevelZero::~LevelZero(){
    for(auto sst: ssts){
        delete sst;
    }
};

std::string LevelZero::Get(uint64_t key, bool *is_failed) const {
    level_0_mutex.lock();
    level_0_mutex.unlock();
}

void LevelZero::MinorCompaction(Utils::ImmutableMemTable& imm_mem) {
    SSTable* new_sst = new SSTable(option, std::string(option.DB_PATH) + std::string("level0.sst.tmp"),
                                   tbl_cache, blk_cache);

    // 1. 把数据从Immutable MemTable中dump到SSTable文件
    new_sst->BuildFromMem(*(imm_mem.sl));
    auto min_key = new_sst->GetMinKey();
    auto max_key = new_sst->GetMaxKey();

    // 2. 添加一个新的Record到manifest文件，并获得文件id
    auto newid = _manifest.CreateSSTRecord(0, min_key, max_key);

    // 3. 重命名文件，并将文件加入到SSTable列表中，删除Immutable MemTable
    new_sst->Rename(newid, std::string(option.DB_PATH) + std::to_string(newid) + ".sst");
    level_0_mutex.lock();
    imm_mem.mutex.lock();
    ssts.push_back(new_sst);
    delete imm_mem.sl;
    imm_mem.sl = nullptr;
    imm_mem.mutex.unlock();
    level_0_mutex.unlock();
}
