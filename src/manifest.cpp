# include "manifest.h"
# include <string>
# include <sstream>

# include <cstdio>
# include <iostream>

Manifest::Manifest(Option &op): option(op){
    std::string path = op.DB_PATH + "manifest.log";
    std::ifstream reader(path, std::ios::in);

    std::string line;

    cur_id = 0;

    if(reader.good()){
        // 读取旧Manifest文件
        while(getline(reader, line)){
            std::istringstream iss(line);
            uint8_t operation;
            iss >> operation;
            switch (operation) {
                case OP_CREATE: {
                    // OP_CREATE level id min_key max_key
                    int level;
                    uint64_t id, min_key, max_key;
                    iss >> level >> id >> min_key >> max_key;

                    record[id] = {level, min_key, max_key};
                }
                    break;
                case OP_DELETE: {
                    // OP_DEL id
                    uint64_t id;
                    iss >> id;
                    record.erase(id);
                }
            }
        }

        reader.close();
        remove(path.c_str());

        // 重写Manifest文件
        writer = new std::ofstream(path, std::ios::out);
        for(auto entry:record){
            cur_id = entry.first;

            // OP_CREATE level id min_key max_key
            (*writer) << OP_CREATE <<" " << entry.second.level << " " << cur_id << " " <<
                entry.second.min_key << " " << entry.second.max_key << std::endl;
        }
    }else {
        writer = new std::ofstream(path, std::ios::out);
        if(!(*writer)){std::cerr<<"Create manifest file failed!"<<std::endl; exit(-1);}
    }
}

Manifest::~Manifest() {
    writer->close();
    delete writer;
}

Manifest::RecordType &Manifest::GetRecord() {
    return record;
}

// 往manifest文件中添加创建文件的记录
uint64_t Manifest::CreateSSTRecord(int level, uint64_t min_key, uint64_t max_key) {
    // OP_CREATE level id min_key max_key
    uint64_t id = (++cur_id);
    (*writer) << OP_CREATE << " " << level << " " << id << " " << min_key << " " << max_key << std::endl;
    return id;
}

// 往manifest文件中添加删除文件的记录
void Manifest::DeleteSSTRecord(uint64_t id) {
    // OP_DEL id
    (*writer) << OP_DELETE << " " << id << std::endl;
}
