#include "db.h"
#include <iostream>
#include <map>
#include <random>

#define INSERT_COUNT 10000000

#define ASSERT_RELEASE(expr) \
if(!(expr)){std::cout << "Assert Failed! In source file"<< __FILE__ << ",line " << __LINE__ << std::endl;exit(-1);}

using namespace std;

// 生成随机字符串
string strRand(int length) {			// length: 产生字符串的长度
    char tmp;							// tmp: 暂存一个随机数
    string buffer;						// buffer: 保存返回值

    // 下面这两行比较重要:
    random_device rd;					// 产生一个 std::random_device 对象 rd
    default_random_engine random(rd());	// 用 rd 初始化一个随机数发生器 random

    for (int i = 0; i < length; i++) {
        tmp = random() % 36;	// 随机一个小于 36 的整数，0-9、A-Z 共 36 种字符
        if (tmp < 10) {			// 如果随机数小于 10，变换成一个阿拉伯数字的 ASCII
            tmp += '0';
        } else {				// 否则，变换成一个大写字母的 ASCII
            tmp -= 10;
            tmp += 'A';
        }
        buffer += tmp;
    }
    return buffer;
}

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
    random_device rd;
    default_random_engine random(rd());

    for(int i=0;i<INSERT_COUNT;i++){
        auto key = random();
        std::string val = strRand(random()%25);
        db.Put(key, val);
        m[key] = val;
    }

    for(int i=0;i<INSERT_COUNT;i++){
        auto key = random();
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

        if(i%100000==0){
            std::cout << "Tested: " << (i*100.0)/INSERT_COUNT << "% OK!" << std::endl;
        }
    }
    return 0;
}


