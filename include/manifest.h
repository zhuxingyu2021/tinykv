#ifndef LSMTREE_MANIFEST_H
#define LSMTREE_MANIFEST_H

# include "option.h"
# include <cstdint>
# include <vector>
# include <map>
# include <fstream>

class Manifest{
public:
    Manifest(Option& op);
    ~Manifest();

    uint64_t CreateSSTRecord(int level, uint64_t min_key, uint64_t max_key);
    void DeleteSSTRecord(uint64_t id);

    struct SSTMetaData{
        int level;
        uint64_t min_key;
        uint64_t max_key;
    };
    // SSTkey -> SSTMetaData
    typedef std::map<uint64_t, SSTMetaData> RecordType;

    RecordType& GetRecord();

    static const uint8_t OP_CREATE=0;
    static const uint8_t OP_DELETE=1;
private:
    uint64_t cur_id;

    Option& option;
    std::ofstream* writer;

    RecordType record; //从Manifest文件中读取出的记录
};

#endif //LSMTREE_MANIFEST_H
