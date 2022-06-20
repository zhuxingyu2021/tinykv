# include "skiplist.h"
# include <cstring>

SkipList::SkipList() : level(0), count(0), memfp_char(0){
    head = new Node;
    head->forward = new Node*[MAX_LEVEL];
    memset(head->forward, 0, sizeof(Node*)*MAX_LEVEL);

    srand(time(0));
}

SkipList::~SkipList() {
    while(head != nullptr){
        auto tmp = head;
        head = head->forward[0];
        delete[] tmp->forward;
        delete tmp;
    }
}

int SkipList::randLevel() {
    int increaseLevel = rand()%2;
    int level = increaseLevel + 1;
    while(increaseLevel && level < MAX_LEVEL){
        increaseLevel = rand()%2;
        level += increaseLevel;
    }
    return level;
}


// 在跳表中插入或更新键值对(key,val)
void SkipList::Put(uint64_t key, const std::string &val) {
    // 1. 查找插入位置
    Node* update[MAX_LEVEL];
    memset(update, 0, sizeof(Node*)*MAX_LEVEL);
    Node* p = head;

    for(int l=level-1;l>=0;l--)
    {
        while(p->forward[l]!= nullptr && p->forward[l]->key<key)
            p = p->forward[l];
        update[l] = p;
    }

    // 2. 插入节点
    p = p->forward[0];
    if(p!=nullptr) {
        if (p->key == key) {
            p->val = val;
            return;
        }
    }
    // 创建节点并插入
    auto lvl = randLevel();
    Node* insertNode = new Node;
    insertNode->key = key;
    insertNode->val = val;
    insertNode->forward = new Node*[lvl];
    memset(insertNode->forward, 0, sizeof(Node*)*lvl);
    memfp_char += insertNode->val.capacity();

    for(int l=0;l<lvl && update[l];l++){
        insertNode->forward[l] = update[l]->forward[l];
        update[l]->forward[l] = insertNode;
    }

    if(lvl > level) {
        for(int l=level;l<lvl;l++)
            head->forward[l] = insertNode;
        level = lvl;
    }
    count++;
}

// 从跳表中获得键key的值，若键key不存在，则返回空字符串
std::string SkipList::Get(uint64_t key) const {
    Node* p = head;
    for(int l=level-1;l>=0;l--)
    {
        while(p->forward[l]!= nullptr && p->forward[l]->key<key)
            p = p->forward[l];
    }
    p = p->forward[0];
    if(p!=nullptr) {
        if (p->key == key) return p->val;
    }
    return "";
}

// 删除跳表中的键值对(key,val)，若成功则返回true，否则返回false
bool SkipList::Del(uint64_t key) {
    // 1. 查找删除位置
    Node* update[MAX_LEVEL];
    memset(update, 0, sizeof(Node*)*MAX_LEVEL);
    Node* p = head;

    for(int l=level-1;l>=0;l--)
    {
        while(p->forward[l]!= nullptr && p->forward[l]->key<key)
            p = p->forward[l];
        update[l] = p;
    }

    p = p->forward[0];
    if(p==nullptr) return false;
    if(p->key!=key) return false;

    // 2. 删除节点p
    int lvl;
    for(lvl=0;lvl<level;lvl++)
    {
        if(update[lvl]->forward[lvl]!=p) break;
        update[lvl]->forward[lvl]=p->forward[lvl];
    }
    memfp_char -= p->val.capacity();
    count--;
    delete[] p->forward;
    delete p;

    // 3. 重新计算跳表层数
    lvl = level-1;
    while(lvl>=0 && head->forward[lvl]== nullptr){
        lvl--;level--;
    }
    return true;
}

// 清空跳表
void SkipList::Clear() {
    Node* p = head->forward[0];
    while(p != nullptr){
        auto tmp = p;
        p = p->forward[0];
        delete[] tmp->forward;
        delete tmp;
    }
    memset(head->forward, 0, sizeof(Node*)*MAX_LEVEL);
    level=0;
    count=0;
    memfp_char=0;
}

// 判断跳表是否为空
bool SkipList::Empty() const {return (head->forward[0] == nullptr);}

// 获得跳表中键值对的个数
size_t SkipList::Size() const {return count;}

// 估计键值对的内存占用
uint64_t SkipList::Space() const {
    return count*sizeof(uint64_t) + memfp_char * sizeof(char);
}
