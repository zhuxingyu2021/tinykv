#include "sstable.h"
#include <iostream>
#include <string>
#include <cstring>
#include "option.h"

SSTable::SSTable(uint64_t id, Cache* tablecache, Cache* blockcache):
tbl_id(id), tbl_cache(tablecache),blk_cache(blockcache){
    path = std::string(Option::DB_PATH) + std::to_string(tbl_id) + ".sst";
    std::ifstream reader(path, std::ios::in | std::ios::binary);
    if(reader.good()){ // 从文件中读取footer信息
        reader.seekg(-FOOTER_SIZE, std::ios::end);
        reader.read((char*)&ib_pos, sizeof(size_t));
        reader.read((char*)&ib_sz, sizeof(size_t));
    }
    reader.close();
}

// SSTable::Get查询失败
#define SST_GET_FAILED if(reader) {reader->close(); delete reader;}\
if(!tbl_cache) delete ib;\
if(!blk_cache) delete db;\
if(is_failed) *is_failed=true;\
return "";

// 从SSTable中获得键key的值并返回。若键key不存在，则is_failed对应的bool变量设置为true，表示查询失败；否则设置为false，表示查询成功。
std::string SSTable::Get(uint64_t key, bool* is_failed) const {
    std::ifstream* reader = nullptr;
    std::map<uint64_t, std::pair<size_t, size_t>>* ib = nullptr;
    std::map<uint64_t,std::string>* db = nullptr;

    // 1. 读取IndexBlock
    if(tbl_cache){
        TableCacheMem* cm = (TableCacheMem*)(tbl_cache->Get(tbl_id, 0));
        if(cm) ib = &cm->ib;
        else{ // TableCache Miss
            reader = new std::ifstream(path, std::ios::in | std::ios::binary);
            if(!reader->good()) {SST_GET_FAILED}

            char* buf_ib = new char[ib_sz];
            reader->seekg(ib_pos, std::ios::beg);
            reader->read(buf_ib, ib_sz);

            cm = (TableCacheMem*)(tbl_cache->Put(tbl_id, 0, buf_ib, ib_sz));
            ib = &cm->ib;
            delete[] buf_ib;
        }
    }else{
        ib = new std::map<uint64_t, std::pair<size_t, size_t>>;

        reader = new std::ifstream(path, std::ios::in | std::ios::binary);
        if(!reader->good()){SST_GET_FAILED}

        char* buf_ib = new char[ib_sz];
        reader->seekg(ib_pos, std::ios::beg);
        reader->read(buf_ib, ib_sz);
        LoadIndexBlockFromBuf(buf_ib, ib_sz, *ib);
        delete[] buf_ib;
    }

    // 2. 从IndexBlock中查找key所在的DataBlock
    auto ib_iter = ib->lower_bound(key);
    // key不存在
    if (ib_iter==ib->end()){SST_GET_FAILED}

    auto db_pos = ib_iter->second.first; // DataBlock的起始地址
    auto db_sz = ib_iter->second.second; // DataBlock的大小

    // 3. 读取DataBlock
    if(blk_cache){
        BlockCacheMem* cm = (BlockCacheMem*)(blk_cache->Get(tbl_id, ib_iter->first));
        if(cm) db = &cm->db;
        else{ // BlockCache Miss
            if(reader== nullptr) reader = new std::ifstream(path, std::ios::in | std::ios::binary);
            if(!reader->good()){SST_GET_FAILED}

            char* buf_db = new char[db_sz];
            reader->seekg(db_pos, std::ios::beg);
            reader->read(buf_db, db_sz);

            cm = (BlockCacheMem*)(blk_cache->Put(tbl_id, ib_iter->first, buf_db, db_sz));
            db = &cm->db;
            delete[] buf_db;
        }
    }
    else{
        db = new std::map<uint64_t,std::string>;
        if(reader== nullptr) reader = new std::ifstream(path, std::ios::in | std::ios::binary);
        if(!reader->good()){SST_GET_FAILED}

        char* buf_db = new char[db_sz];
        reader->seekg(db_pos, std::ios::beg);
        reader->read(buf_db, db_sz);
        LoadDataBlockFromBuf(buf_db, db_sz, *db);
        delete[] buf_db;
    }

    // 4. 在DataBlock中查找key
    auto db_iter = db->find(key);

    // key不存在
    if(db_iter==db->end()){SST_GET_FAILED}

    auto val = db_iter->second;
    if(reader) {reader->close(); delete reader;}
    if(!tbl_cache) delete ib;
    if(!blk_cache) delete db;
    if(is_failed) *is_failed=false;
    return val;
}

// Minor Compaction
void SSTable::BuildFromMem(SkipList &sl) {
    std::ofstream sstfile(path, std::ios::out | std::ios::binary);

    // 1. 构造DataBlock和IndexBlock，同时往文件中写入DataBlock
    char* db = new char[Option::DATA_BLOCK_SIZE];
    int offset_db = 0;
    int dbcount = 0;

    size_t buffersz = Option::BUFFER_SIZE;
    char* buffer_ib = new char[buffersz];
    int offset_ib = 0;
    int maxkey; // IndexBlock的索引
    size_t pos_db = sstfile.tellp();
    size_t entrysize_ib = sizeof(uint64_t) + 2*sizeof(size_t);

    for(auto kv:sl){
        size_t lenval = kv.second.length();
        size_t entrysize_db = lenval * sizeof(char) + sizeof(size_t) + sizeof(uint64_t);
        if(offset_db + entrysize_db > Option::DATA_BLOCK_SIZE){ // 当前DataBlock已满
            sstfile.write(db, offset_db);
            offset_db = 0;
            dbcount++;
            size_t nowpos = sstfile.tellp();

            if(offset_ib + entrysize_ib > buffersz){
                char* new_buffer = new char[buffersz*2];
                memcpy(new_buffer, buffer_ib, offset_ib);
                buffersz *= 2;
                delete[] buffer_ib;
                buffer_ib = new_buffer;
            }
            *((uint64_t*)(buffer_ib + offset_ib)) = maxkey; // IndexBlock Entry第一项：DataBlock最大值
            offset_ib += sizeof(uint64_t);
            *((size_t*)(buffer_ib + offset_ib)) = pos_db; // IndexBlock Entry第二项：DataBlock起始地址
            offset_ib += sizeof(size_t);
            *((size_t*)(buffer_ib + offset_ib)) = nowpos - pos_db; // IndexBlock Entry第三项：DataBlock长度
            offset_ib += sizeof(size_t);

            pos_db = nowpos;
        }
        maxkey = kv.first;
        *((size_t*)(db + offset_db)) = entrysize_db; //DataBlock Entry第一项：entry大小
        offset_db += sizeof(size_t);
        *((uint64_t*)(db + offset_db)) = maxkey; //DataBlock Entry第二项：key
        offset_db += sizeof(uint64_t);
        memcpy(db + offset_db, kv.second.c_str(), lenval); //DataBlock Entry第三项：value
        offset_db += lenval;
    }
    if(offset_db > 0){
        sstfile.write(db, offset_db);
        offset_db = 0;
        dbcount++;
        size_t nowpos = sstfile.tellp();

        if(offset_ib + entrysize_ib > buffersz){
            char* new_buffer = new char[buffersz*2];
            memcpy(new_buffer, buffer_ib, offset_ib);
            buffersz *= 2;
            delete[] buffer_ib;
            buffer_ib = new_buffer;
        }
        *((uint64_t*)(buffer_ib + offset_ib)) = maxkey; // IndexBlock Entry第一项：DataBlock最大值
        offset_ib += sizeof(uint64_t);
        *((size_t*)(buffer_ib + offset_ib)) = pos_db; // IndexBlock Entry第二项：DataBlock起始地址
        offset_ib += sizeof(size_t);
        *((size_t*)(buffer_ib + offset_ib)) = nowpos - pos_db; // IndexBlock Entry第三项：DataBlock长度
        offset_ib += sizeof(size_t);

        pos_db = nowpos;
    }

    // 2. 向文件中写入IndexBlock
    ib_pos = pos_db; // IndexBlock的起始位置
    ib_sz = offset_ib; // IndexBlock的大小
    sstfile.write(buffer_ib, offset_ib);
    if(tbl_cache){
        tbl_cache->Put(tbl_id, 0, buffer_ib, offset_ib);
    }

    // 3. 写入footer信息
    sstfile.write((char*)&ib_pos, sizeof(size_t));
    sstfile.write((char*)&ib_sz, sizeof(size_t));

    delete[] db;
    delete[] buffer_ib;
    sstfile.close();
}

// 从文件读取的内存缓冲区中加载DataBlock
void SSTable::LoadDataBlockFromBuf(char *buf, size_t bufsz, std::map<uint64_t,std::string>& db) {
    size_t offset = 0;
    do{
        size_t entrysize_db = *((uint64_t*)(buf+offset));
        size_t str_sz = entrysize_db-(sizeof(size_t)+sizeof(uint64_t));
        db[*((uint64_t*)(buf+offset+sizeof(size_t)))] = std::string(buf+offset+sizeof(size_t)+sizeof(uint64_t), str_sz);
        offset += entrysize_db;
    }while(offset<bufsz);
}

// 从文件读取的内存缓冲区中加载IndexBlock
void SSTable::LoadIndexBlockFromBuf(char *buf, size_t bufsz, std::map<uint64_t, std::pair<size_t, size_t>> &ib) {
    size_t offset = 0;
    size_t entrysize_ib = sizeof(uint64_t) + 2*sizeof(size_t);
    do{
        ib[*((uint64_t*)(buf+offset))] = std::make_pair(*((size_t*)(buf+offset+sizeof(uint64_t))),
                                                        *((size_t*)(buf+offset+sizeof(uint64_t)+sizeof(size_t))));
        offset += entrysize_ib;
    }while(offset<bufsz);
}
