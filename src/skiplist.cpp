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
        if (p->key == key) { // 更新节点
            memfp_char -= p->val.capacity();
            memfp_char += val.capacity();
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

// 从跳表中获得键key的值并返回。若键key不存在，则is_failed对应的bool变量设置为true，表示查询失败；否则设置为false，表示查询成功。
std::string SkipList::Get(uint64_t key, bool* is_failed) const {
    Node* p = head;
    for(int l=level-1;l>=0;l--)
    {
        while(p->forward[l]!= nullptr && p->forward[l]->key<key)
            p = p->forward[l];
    }
    p = p->forward[0];
    if(p!=nullptr) {
        if (p->key == key) {
            if(is_failed) *is_failed=false;
            return p->val;
        }
    }
    if(is_failed) *is_failed=true;
    return "";
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
size_t SkipList::Space() const {
    return count*sizeof(size_t) + memfp_char * sizeof(char);
}
