#pragma once
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
