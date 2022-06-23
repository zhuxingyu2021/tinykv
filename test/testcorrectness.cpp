#include "db.h"
#include <iostream>
#include <map>

#define INSERT_COUNT 500000
#define RANGE_MAX 1000000

#define ASSERT_RELEASE(expr) \
if(!(expr)){std::cout << "Assert Failed! In source file"<< __FILE__ << ",line " << __LINE__ << std::endl;exit(-1);}

int main()
{
    DB db;
    std::map<int,std::string> m;
    for(int i=0;i<INSERT_COUNT;i++){
        auto key = rand()%RANGE_MAX;
        std::string val = std::to_string(key+rand()%RANGE_MAX);
        db.Put(key, val);
        m[key] = val;
    }

    for(int i=0;i<INSERT_COUNT;i++){
        auto key = rand()%RANGE_MAX;
        auto dbval = db.Get(key);
        auto miter = m.find(key);
        if(miter!=m.end()){
            auto mval = miter->second;
            ASSERT_RELEASE(dbval==mval);
            db.Del(key);
            m.erase(miter);
        }else{
            ASSERT_RELEASE(dbval.length()==0);
        }

        if(i%1000==0){
            std::cout << "Tested: " << (i*100.0)/INSERT_COUNT << "%" << std::endl;
        }
    }
    return 0;
}


