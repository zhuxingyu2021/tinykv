// 测试skiplist的功能

#include "skiplist.h"
#include <iostream>
#include <map>

#define INSERT_COUNT 400
#define RANGE_MAX 300

#define ASSERT_RELEASE(expr) \
if(!(expr)){std::cout << "Assert Failed! In source file"<< __FILE__ << ",line " << __LINE__ << std::endl;exit(-1);}

int main()
{
    SkipList sl;
    std::map<int,std::string> m;
    for(int i=0;i<INSERT_COUNT;i++){
        auto key = rand()%RANGE_MAX;
        std::string val = std::to_string(key+rand()%RANGE_MAX);
        sl.Put(key, val);
        m[key] = val;
    }
    std::cout << "Size: " << sl.Size() << std::endl;
    std::cout << "Space: " << sl.Space() << std::endl;

    for(auto kv:sl){
        std::cout << "key= " << kv.first << ",val =" << kv.second << std::endl;
    }

    for(int i=0;i<INSERT_COUNT;i++){
        auto key = rand()%RANGE_MAX;
        auto slval = sl.Get(key);
        auto miter = m.find(key);
        if(miter!=m.end()){
            auto mval = miter->second;
            ASSERT_RELEASE(slval==mval);
            ASSERT_RELEASE(sl.Del(key));
            m.erase(miter);
        }
        else{
            ASSERT_RELEASE(!sl.Del(key));
        }
    }

    std::cout << "Size: " << sl.Size() << std::endl;
    std::cout << "Space: " << sl.Space() << std::endl;
    return 0;
}
