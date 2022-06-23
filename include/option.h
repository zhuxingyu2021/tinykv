#ifndef LSMTREE_OPTION_H
#define LSMTREE_OPTION_H

#include <cstddef>
#include <string>

struct Option{
    Option():DB_PATH("./database/"){}

    int MAX_LEVEL=8;

    size_t MAX_MEMTABLE_SIZE=1024*1024*2;

    size_t DATA_BLOCK_SIZE=4096;
    size_t BUFFER_SIZE=4096;
    size_t BLOCKCACHE_SIZE=1024*1024*16;
    size_t TABLECACHE_SIZE=1024*1024;

    bool BLOCKCACHE_ENABLED=true;
    bool TABLECHCHE_ENABLED=true;

    std::string DB_PATH;
};

#endif //LSMTREE_OPTION_H
