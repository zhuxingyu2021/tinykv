#ifndef LSMTREE_OPTION_H
#define LSMTREE_OPTION_H

struct Option{
    size_t MAX_MEMTABLE_SIZE=1024*32;

    size_t DATA_BLOCK_SIZE=4096;
    size_t BUFFER_SIZE=4096;
    size_t BLOCKCACHE_SIZE=1024*1024*2;
    size_t TABLECACHE_SIZE=1024*512;

    bool BLOCKCACHE_ENABLED=true;
    bool TABLECHCHE_ENABLED=true;

    char* DB_PATH=(char*)"./database/";
};

#endif //LSMTREE_OPTION_H
