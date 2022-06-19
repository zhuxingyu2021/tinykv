#ifndef MYLSMTREE_SKIPLIST_H
#define MYLSMTREE_SKIPLIST_H

#include <cstdint>
#include <cstddef>
#include <string>

class SkipList{
public:
    explicit SkipList();
    ~SkipList();
    void put(uint64_t key, const std::string& val);
    std::string get(uint64_t key) const;
    bool del(uint64_t key);

    void clear();

    bool empty() const;
    size_t size() const;
    uint64_t space() const;

    const int MAX_LEVEL = 20; // 跳表的最大层数
private:
    int level; // 跳表层数
    size_t count; // 跳表元素计数
    uint64_t memfpchar; // 用以估计字符串的内存占用

    struct Node{ // 跳表节点类型定义
        uint64_t key;
        std::string val;
        Node** forward;
    };

    Node* head; // 跳表首节点

    int randLevel(); // 在创建节点时生成随机的层数
};


#endif //MYLSMTREE_SKIPLIST_H
