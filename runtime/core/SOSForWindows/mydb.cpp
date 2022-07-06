/*
模块名称：数据库模块
实现功能：处理用户信息
完成作者：洪铱宁
测试人员：郭明皓 史家旭
审核人员：郭明皓 刘元庆
*/
#include "mydb.h"
#include "MyDB.h"
#include "MyMD5.h"

#include <fstream>
#include <vector>
#include <utility>

int search_DB(string usename, string pwd) {
    fstream DB;
    DB.open(DB_FILE_NAME);

    string un, pw;
    while (DB >> un >> pw) {
        if (un == usename) {
            DB.close();
            return pw == pwd ? 1 : -1;
        }
    }

    DB.close();
    return 0;
}

int insert_DB(string usename, string pwd) {
    vector<pair<string, string> > temp;
    fstream DB;
    DB.open(DB_FILE_NAME);

    string un, pw;
    while (DB >> un >> pw) {
        temp.emplace_back(make_pair(un, pw));
        if (un == usename) {
            DB.close();
            return -1;
        }
    }

    DB.close();
    DB.open(DB_FILE_NAME);
    for (int i = 0; i < temp.size(); i++) {
        DB << temp[i].first << endl << temp[i].second << endl;
    }

    DB << usename << endl << pwd << endl;
    DB.close();

    return 1;
}

void init() {
    ofstream DB1(DB_FILE_NAME, iostream::out);
    DB1.close();

    fstream DB;
    DB.open(DB_FILE_NAME);

    DB << "test1" << endl << getMD5("test1") << endl;
    DB << "test2" << endl << getMD5("test2") << endl;
    DB << "test3" << endl << getMD5("test3") << endl;

    DB.close();
}

void test() {
    cout << insert_DB("text4", getMD5("test4")) << endl;
    cout << insert_DB("text4", "1") << endl;
    cout << search_DB("test1", getMD5("test1")) << endl;
    cout << search_DB("test1", getMD5("test2")) << endl;
    cout << search_DB("test5", getMD5("test1")) << endl;
}
