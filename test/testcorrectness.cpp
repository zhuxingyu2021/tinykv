#include "db.h"
#include <iostream>
#include <map>

#define INSERT_COUNT 200000
#define RANGE_MAX 400000

#define ASSERT_RELEASE(expr) \
if(!(expr)){std::cout << "Assert Failed! In source file"<< __FILE__ << ",line " << __LINE__ << std::endl;exit(-1);}

int main()
{
    std::map<int,std::string> m;

    Option default_option;

#ifdef linux
    std::string command = std::string("rm -rf ") + default_option.DB_PATH;
    system(command.c_str());
#else
    std::cerr << "The test code must run in linux!" << std::endl;
    exit(-1);
#endif

    DB db(default_option);

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


