#ifndef LSMTREE_SKIPLIST_H
#define LSMTREE_SKIPLIST_H

#include <cstdint>
#include <cstddef>
#include <string>

class SkipList{
public:
    explicit SkipList();
    ~SkipList();
    void Put(uint64_t key, const std::string& val);
    std::string Get(uint64_t key, bool* is_failed) const;
    bool Del(uint64_t key);

    void Clear();

    bool Empty() const;
    size_t Size() const;
    uint64_t Space() const;


    struct Node{ // 跳表节点类型定义
        uint64_t key;
        std::string val;
        Node** forward;
    };

    struct Iterator{ // 自定义迭代器
    public:
        Iterator(Node* n):p(n){}
        std::pair<uint64_t, std::string&> operator*() const{return std::pair<uint64_t, std::string&>(p->key, p->val);}
        bool operator!=(const Iterator& rhs) const{return (p!=rhs.p);}
        Iterator& operator++(void){p=p->forward[0];return (*this);};
    private:
        Node* p;
    };
    Iterator begin() const{return Iterator(head->forward[0]);}
    Iterator end() const{return Iterator(nullptr);}

    static const int MAX_LEVEL = 20; // 跳表的最大层数
private:
    int level; // 跳表层数
    size_t count; // 跳表元素计数
    uint64_t memfp_char; // 用以估计字符串的内存占用

    Node* head; // 跳表首节点

    int randLevel(); // 在创建节点时生成随机的层数
};


#endif //LSMTREE_SKIPLIST_H
