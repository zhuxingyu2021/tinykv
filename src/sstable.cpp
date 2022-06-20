#include "sstable.h"
#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include "option.h"

SSTable::SSTable(uint64_t id, Cache* tablecache, Cache* blockcache):
tbl_id(id), tbl_cache(tablecache),blk_cache(blockcache),loaded(false){
    path = std::string(Option::DB_PATH) + std::to_string(tbl_id) + ".sst";
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
        maxkey = kv.first;
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
    sstfile.write(buffer_ib, offset_ib);
    if(tbl_cache){
        tbl_cache->Put(tbl_id, 0, buffer_ib, offset_ib);
    }

    // 3. 写入footer信息
    sstfile.write((char*)&ib_pos, sizeof(size_t));

    delete[] db;
    delete[] buffer_ib;
    sstfile.close();

    loaded = true;
}

void SSTable::LoadDataBlockFromBuf(char *buf, size_t bufsz, std::map<uint64_t,std::string>& db) {
    // TODO
}

void SSTable::LoadIndexBlockFromBuf(char *buf, size_t bufsz, std::map<uint64_t, std::pair<size_t, size_t>> &ib) {
    // TODO
}
