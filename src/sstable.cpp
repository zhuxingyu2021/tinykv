#include "sstable.h"
#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include <cassert>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

// 构建新的SSTable
SSTable::SSTable(Option& op, std::string&& path, Cache* tablecache, Cache* blockcache):
    path(path),tbl_cache(tablecache),blk_cache(blockcache), option(op),writer(nullptr),buf_ib(nullptr),buf_db(nullptr),
    ib_sz(0),ib_pos(0),first_write(true),offset_db(0),offset_ib(0),pos_db(0){}

// 从文件构造SSTable
SSTable::SSTable(Option &op, uint64_t id, std::string &&path, Cache *tablecache, Cache *blockcache, uint64_t minkey, uint64_t maxkey):
tbl_id(id),path(path),tbl_cache(tablecache),blk_cache(blockcache), option(op), min_key(minkey), max_key(maxkey),writer(
        nullptr),buf_db(nullptr),buf_ib(nullptr),ib_sz(0),ib_pos(0),first_write(true),offset_db(0),offset_ib(0),pos_db(0){
    std::ifstream reader(path, std::ios::in | std::ios::binary);
    if(reader.good()){ // 从文件中读取footer信息
        reader.seekg(-FOOTER_SIZE, std::ios::end);
        reader.read((char*)&ib_pos, sizeof(size_t));
        reader.read((char*)&ib_sz, sizeof(size_t));
    }else{
        std::cerr << "SSTable " << path << " can't find!" << std::endl;
        exit(-1);
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
    CreateSSTFile();

    for(auto kv:sl){
        WriteDataBlock(kv.first, kv.second);
    }

    WriteMetaData();
}

// 创建新的SSTable
void SSTable::CreateSSTFile() {
    buf_db = nullptr; buf_ib = nullptr;
    offset_db = 0;
    offset_ib = 0;
    pos_db = 0;
    first_write = true;

    if(writer){std::cerr<<"SSTable already created!"<<std::endl; exit(-1);}
    writer = new std::ofstream(path, std::ios::out | std::ios::binary);
    if(!(*writer)){std::cerr<<"Create SSTable file failed!"<<std::endl; exit(-1);}
}

// 将(key, value)写入DataBlock
void SSTable::WriteDataBlock(uint64_t key, const std::string &value) {
    if(!buf_db) buf_db = new char[option.DATA_BLOCK_SIZE];
    if(!buf_ib) {
        buf_ib_sz = option.BUFFER_SIZE;
        buf_ib = new char[buf_ib_sz];
    }

    size_t entrysize_ib = sizeof(uint64_t) + 2*sizeof(size_t);
    if(first_write){
        min_key = key;
        first_write = false;
    }

    size_t lenval = value.length();
    size_t entrysize_db = lenval * sizeof(char) + sizeof(size_t) + sizeof(uint64_t);
    if(offset_db + entrysize_db > option.DATA_BLOCK_SIZE){ // 当前DataBlock已满
        writer->write(buf_db, offset_db);
        offset_db = 0;
        size_t nowpos = writer->tellp();

        if(offset_ib + entrysize_ib > buf_ib_sz){
            char* new_buffer = new char[buf_ib_sz*2];
            memcpy(new_buffer, buf_ib, offset_ib);
            buf_ib_sz *= 2;
            delete[] buf_ib;
            buf_ib = new_buffer;
        }
        *((uint64_t*)(buf_ib + offset_ib)) = max_db_key; // IndexBlock Entry第一项：DataBlock键值最大值
        offset_ib += sizeof(uint64_t);
        *((size_t*)(buf_ib + offset_ib)) = pos_db; // IndexBlock Entry第二项：DataBlock起始地址
        offset_ib += sizeof(size_t);
        *((size_t*)(buf_ib + offset_ib)) = nowpos - pos_db; // IndexBlock Entry第三项：DataBlock长度
        offset_ib += sizeof(size_t);

        pos_db = nowpos;
    }
    max_db_key = key;
    *((size_t*)(buf_db + offset_db)) = entrysize_db; //DataBlock Entry第一项：entry大小
    offset_db += sizeof(size_t);
    *((uint64_t*)(buf_db + offset_db)) = max_db_key; //DataBlock Entry第二项：key
    offset_db += sizeof(uint64_t);
    memcpy(buf_db + offset_db, value.c_str(), lenval); //DataBlock Entry第三项：value
    offset_db += lenval;
}

// 将IndexBlock写入文件并写入footer信息
void SSTable::WriteMetaData() {
    size_t entrysize_ib = sizeof(uint64_t) + 2*sizeof(size_t);

    // 将缓冲区中的DataBlock写入文件
    if(offset_db > 0){
        writer->write(buf_db, offset_db);
        size_t nowpos = writer->tellp();

        if(offset_ib + entrysize_ib > buf_ib_sz){
            char* new_buffer = new char[buf_ib_sz*2];
            memcpy(new_buffer, buf_ib, offset_ib);
            buf_ib_sz *= 2;
            delete[] buf_ib;
            buf_ib = new_buffer;
        }
        *((uint64_t*)(buf_ib + offset_ib)) = max_db_key; // IndexBlock Entry第一项：DataBlock键值最大值
        offset_ib += sizeof(uint64_t);
        *((size_t*)(buf_ib + offset_ib)) = pos_db; // IndexBlock Entry第二项：DataBlock起始地址
        offset_ib += sizeof(size_t);
        *((size_t*)(buf_ib + offset_ib)) = nowpos - pos_db; // IndexBlock Entry第三项：DataBlock长度
        offset_ib += sizeof(size_t);

        pos_db = nowpos;
    }
    max_key = max_db_key;

    // 向文件中写入IndexBlock
    ib_pos = pos_db; // IndexBlock的起始位置
    ib_sz = offset_ib; // IndexBlock的大小
    writer->write(buf_ib, offset_ib);

    // 写入footer信息
    writer->write((char*)&ib_pos, sizeof(size_t));
    writer->write((char*)&ib_sz, sizeof(size_t));

    delete[] buf_db;
    delete[] buf_ib;
    writer->close();
    delete writer;

    buf_db = nullptr; buf_ib = nullptr;
    offset_db = 0;
    offset_ib = 0;
    pos_db = 0;
    first_write = true;
}

// 重命名SSTable文件
void SSTable::Rename(uint64_t newid, std::string &&newpath) {
    std::string old_path = path;
    path = newpath;
    tbl_id = newid;

    rename(old_path.c_str(), path.c_str());
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

IterableSSTable::IterableSSTable(const SSTable &sst, const Option& option){
    MAX_BUF_SZ = option.BUFFER_SIZE;

    reader = new std::ifstream(sst.GetPath(), std::ios::in | std::ios::binary);
    if(!reader->good()){std::cerr << "SSTable " << sst.GetPath() << " can't find!" << std::endl; exit(1);}

    ib_pos = sst.GetIndexBlockPosition();
    buf = nullptr;

    default_iterator.init(this, 0);
}

IterableSSTable::~IterableSSTable() {
    reader->close();
    delete reader;
    delete[] buf;
}

// 根据当前迭代器的位置，把文件内容加载到缓冲区
#define LOAD_BUFFER(pos) _it->reader->seekg((pos));\
    _it->buf_sz = MIN(_it->MAX_BUF_SZ, _it->ib_pos - (pos));\
    delete[] _it->buf;\
    _it->buf = new char[_it->buf_sz];\
    _it->reader->read(_it->buf, _it->buf_sz);\
    _it->buf_pos = (pos);

IterableSSTable::Iterator::Iterator(IterableSSTable *it, size_t pos){
    init(it, pos);
}

void IterableSSTable::Iterator::init(IterableSSTable *it, size_t pos) {
    _it = it;
    file_pos = pos;

    // 当前迭代器位置不在buffer的范围内
    if((!_it->buf) || (file_pos < _it->buf_pos) || (file_pos >= _it->buf_pos + _it->buf_sz)) {
        LOAD_BUFFER(file_pos);
    }
    offset = file_pos - _it->buf_pos;
}

IterableSSTable::Iterator& IterableSSTable::Iterator::operator++() {
    // DataBlock Entry第一项：entry大小 (size_t)

    // 跳到下一个Entry
    file_pos += *((size_t*)(_it->buf + offset));
    offset += *((size_t*)(_it->buf + offset));

    // 重新加载buffer
    if(file_pos < _it->ib_pos) {
        size_t entrysize;
        if (_it->buf_sz - offset <= sizeof(size_t)) {
            LOAD_BUFFER(file_pos);
            offset = 0;
        }
        entrysize = *((size_t*)(_it->buf + offset));
        if (_it->buf_sz - offset <= entrysize) {
            LOAD_BUFFER(file_pos);
            offset = 0;
        }
    }
    return *this;
}

std::pair<uint64_t, std::string> IterableSSTable::Iterator::operator*() const {
    // DataBlock Entry第一项：entry大小 (size_t)
    // DataBlock Entry第二项：key (uint64_t)
    // DataBlock Entry第三项：value (std::string)

    if(file_pos < _it->ib_pos) {
        size_t entrysize = *((size_t*)(_it->buf + offset));
        uint64_t key = *((uint64_t*)(_it->buf + offset + sizeof(size_t)));
        auto str_sz = entrysize - sizeof(size_t) - sizeof(uint64_t);
        return std::make_pair(key, std::string(_it->buf + offset + sizeof(size_t) + sizeof(uint64_t), str_sz));
    }else{
        return std::make_pair(0, "");
    }
}