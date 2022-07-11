# include <iostream>
# include <map>
# include <random>
# include <chrono>
# include "db.h"

using namespace std;

#define INSERT_COUNT 8000000

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

int main(){
    std::map<uint64_t ,std::string> m;

    Option default_option;

#ifdef linux
    std::string command = std::string("rm -rf ") + default_option.DB_PATH;
    system(command.c_str());
#else
    std::cerr << "The test code must run in linux!" << std::endl;
    exit(-1);
#endif

    DB db;
    random_device rd;
    default_random_engine random(rd());

    for(int i=0;i<INSERT_COUNT;i++){
        auto key = random();
        std::string val = strRand(25);
        m[key] = val;
    }

    // 测试顺序写性能
    auto start_put = chrono::system_clock::now();
    for(auto kv:m){
        db.Put(kv.first, kv.second);
    }
    auto end_put = chrono::system_clock::now();
    auto duration_put = chrono::duration_cast<chrono::microseconds>(end_put - start_put);
    double second_put = double(duration_put.count()) * chrono::microseconds::period::num / chrono::microseconds::period::den;

    cout <<  "SEQ Write: " << INSERT_COUNT/second_put << " OPS" << endl;

    // 测试顺序读性能
    auto start_get = chrono::system_clock::now();
    for(auto kv:m){
        db.Get(kv.first);
    }
    auto end_get = chrono::system_clock::now();
    auto duration_get = chrono::duration_cast<chrono::microseconds>(end_get - start_get);
    double second_get = double(duration_get.count()) * chrono::microseconds::period::num / chrono::microseconds::period::den;

    cout <<  "SEQ Read: " << m.size()/second_get << " OPS" << endl;

}

