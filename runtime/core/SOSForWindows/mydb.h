/*
模块名称：数据库模块
实现功能：处理用户信息
完成作者：洪铱宁
测试人员：郭明皓 史家旭
审核人员：郭明皓 刘元庆
*/
#ifndef MYDB_H
#define MYDB_H
#include<string>
using namespace std;
#include <iostream>

#define DB_FILE_NAME "DB.txt"

class MYDB
{
public:
    MYDB();
    int search_DB(string usename, string pwd);
    int insert_DB(std::string usename, string pwd);

    void init();
    void test();
};

#endif // MYDB_H
