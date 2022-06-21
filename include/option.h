#ifndef LSMTREE_OPTION_H
#define LSMTREE_OPTION_H

struct Option{
    static const size_t DATA_BLOCK_SIZE=4096;
    static const size_t BUFFER_SIZE=4096;
    static const size_t BLOCKCACHE_SIZE=4096*16;
    static const size_t TABLECACHE_SIZE=16384;

    static const bool BLOCKCACHE_ENABLED=true;
    static const bool TABLECHCHE_ENABLED=true;

    static constexpr char* DB_PATH=(char*)"./";
};

#endif //LSMTREE_OPTION_H
