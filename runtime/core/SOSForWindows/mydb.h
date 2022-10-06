
#pragma once
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
