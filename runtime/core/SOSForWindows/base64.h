/*
模块名称：加密模块
实现功能：信息加密
完成作者：史家旭
测试人员：郭明皓 史家旭
审核人员：郭明皓，刘元庆
*/
#ifndef BASE64_H
#define BASE64_H

#include "iostream"
#include <string>
#include <assert.h>
using namespace std;

typedef unsigned char uchar;

enum Base64Option {
    Base64Encoding = 0,
    Base64UrlEncoding = 1,

    KeepTrailingEquals = 0,
    OmitTrailingEquals = 2
};

string encode_base64(const string sourceData, int options = Base64Encoding);
string decode_base64(const string sourceData, int options = Base64Encoding);


#endif // BASE64_H
