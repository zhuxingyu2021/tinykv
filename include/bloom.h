#ifndef LSMTREE_BLOOM_H
#define LSMTREE_BLOOM_H

#include <array>
#include <fstream>
#include "option.h"

class BloomFilter{
public:
    // 将val加入集合中
    void Add(uint64_t val){
        uint64_t hash = murmur3_64(val);
        uint a = (uint) (hash >> 32);
        uint b = (uint) hash;
        for (int i = 0; i < Option::BLOOM_FUNC_CNT; i++) {
            // bloom_bitset.set(reduce(a, Option::BLOOM_BITS));
            uint32_t set_bit = reduce(a, Option::BLOOM_BITS);
            bloom_bitset[set_bit/8] = bloom_bitset[set_bit/8] | (1 << (set_bit % 8));
            a += b;
        }
    }

    // 判断val是否不存在
    bool NotExist(uint64_t val) const{
        uint64_t hash = murmur3_64(val);
        uint a = (uint) (hash >> 32);
        uint b = (uint) hash;
        for (int i = 0; i < Option::BLOOM_FUNC_CNT; i++) {
            uint32_t set_bit = reduce(a, Option::BLOOM_BITS);
            if(bloom_bitset[set_bit/8] & (1 << (set_bit % 8))){
                a += b;
            }else{
                return true;
            }
        }
        return false;
    }

    size_t WriteToFile(std::ofstream& writer){
        writer.write(reinterpret_cast<const char *>(bloom_bitset.data()), Option::BLOOM_BITS / 8);
        return Option::BLOOM_BITS/8;
    }

    bool ReadFromFile(std::ifstream& reader, size_t sz){
        if(sz != Option::BLOOM_BITS / 8)
            return false;
        reader.read(reinterpret_cast<char *>(bloom_bitset.data()), Option::BLOOM_BITS / 8);
        return true;
    }
private:
    uint64_t murmur3_64(uint64_t x) const{
        x = (x ^ (x >> 33)) * 0xff51afd7ed558ccdL;
        x = (x ^ (x >> 23)) * 0xc4ceb9fe1a85ec53L;
        x = x ^ (x >> 33);
        return x;
    }

    uint32_t reduce(uint32_t x, uint32_t N) const{
        // https://lemire.me/blog/2016/06/27/a-fast-alternative-to-the-modulo-reduction/
        return ((uint64_t) x * (uint64_t) N) >> 32 ;
    }

    std::array<uint8_t, Option::BLOOM_BITS/8> bloom_bitset;
};

#endif //LSMTREE_BLOOM_H
