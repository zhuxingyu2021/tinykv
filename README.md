# Tinykv：A simple KV storage system based on LSM-Tree
**基于LSM-Tree的键值存储系统**

## 构建 / Build
```shell
mkdir build
cd build
cmake ..
make
```
你可以看到编译后的动态链接库liblsmtree.so以及一些测试案例  

## 进度 / Progress
已完成的部分：
- [x] Skiplist
- [x] Storage
- [x] Compaction
- [x] Scheduling
- [x] Cache
- [x] BloomFilter
- [x] Version

## 反馈与参与
* Bug、建议都欢迎提在[Issues](https://github.com/zhuxingyu2021/tinykv/issues)或者发送邮件至wode057406422181@gmail.com