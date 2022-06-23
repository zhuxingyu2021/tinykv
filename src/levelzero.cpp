#include "levelzero.h"

LevelZero::LevelZero(Option& op, Manifest& manifest,Cache* tablecache, Cache* blockcache): tbl_cache(tablecache),
blk_cache(blockcache), option(op), _manifest(manifest), empty(true){}

LevelZero::LevelZero(Option& op, Manifest& manifest, Utils::LevelMetaDataType& levelmetadata, Cache* tablecache, Cache* blockcache):
tbl_cache(tablecache), blk_cache(blockcache), option(op), _manifest(manifest), empty(false){
    for(auto sst: levelmetadata){
        auto id = sst.first;
        ssts.push_back(new SSTable(op, id, op.DB_PATH + std::to_string(id) + std::string(".sst"),
                                   tablecache, blockcache, sst.second.first, sst.second.second));
    }
}

LevelZero::~LevelZero(){
    for(auto sst: ssts){
        delete sst;
    }
};

// 从Level 0中获得键key的值并返回。若键key不存在，则is_failed对应的bool变量设置为true，表示查询失败；否则设置为false，表示查询成功。
std::string LevelZero::Get(uint64_t key, bool *is_failed) const {
    bool failed;
    std::string val;
    std::shared_lock Lock(level_0_mutex);
    if(empty){if(*is_failed) *is_failed = true; return "";} // 如果sst文件不存在
    for(auto iter=ssts.rbegin(); iter!=ssts.rend(); iter++){
        auto sst = *iter;
        if((sst->GetMinKey()<=key) && (sst->GetMaxKey()>=key)){
            val = sst->Get(key, &failed);
            if(!failed){if(is_failed) *is_failed=false; return val;}
        }
    }
    if(is_failed) *is_failed=true;
    return "";
}

// 获得当前Level的所有SSTable
const std::vector<SSTable*>& LevelZero::GetSSTables() const {
    return ssts;
}

// 执行MinorCompaction
void LevelZero::MinorCompaction(Utils::ImmutableMemTable& imm_mem) {
    SSTable* new_sst = new SSTable(option, option.DB_PATH + std::string("level0.sst.tmp"),
                                   tbl_cache, blk_cache);

    // 1. 把数据从Immutable MemTable中dump到SSTable文件
    new_sst->BuildFromMem(*(imm_mem.sl));
    auto min_key = new_sst->GetMinKey();
    auto max_key = new_sst->GetMaxKey();

    // 2. 添加一个新的Record到manifest文件，并获得文件id
    auto newid = _manifest.CreateSSTRecord(0, min_key, max_key);

    // 3. 重命名文件，并将文件加入到SSTable列表中，删除Immutable MemTable
    new_sst->Rename(newid, option.DB_PATH + std::to_string(newid) + ".sst");
    std::unique_lock Lock1(level_0_mutex);
    std::unique_lock Lock2(imm_mem.mutex);
    ssts.push_back(new_sst);
    empty = false;
    delete imm_mem.sl;
    imm_mem.sl = nullptr;
}
