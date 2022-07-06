/*
模块名称：加密模块
实现功能：信息加密
完成作者：史家旭
测试人员：郭明皓 史家旭
审核人员：郭明皓，刘元庆
*/
#include "base64.h"
#include "iostream"
#include "assert.h"
#include "base64.h"
#include<string>

std::string encode_base64(const string sourceData, int options) {
    const char alphabet_base64[] = "ABCDEFGH" "IJKLMNOP" "QRSTUVWX" "YZabcdef" "ghijklmn" "opqrstuv" "wxyz0123" "456789+/";
    const char alphabet_base64url[] = "ABCDEFGH" "IJKLMNOP" "QRSTUVWX" "YZabcdef" "ghijklmn" "opqrstuv" "wxyz0123" "456789-_";
    const char* const alphabet = options & Base64UrlEncoding ? alphabet_base64url : alphabet_base64;
    const char padchar = '=';
    int padlen = 0;

    string tmp;
    tmp.resize((sourceData.size() + 2) / 3 * 4);

    int i = 0;
    char* out = &tmp[0];
    while (i < sourceData.size()) {
        // encode 3 bytes at a time
        int chunk = 0;
        chunk |= int(uchar(sourceData.data()[i++])) << 16;
        if (i == sourceData.size())
            padlen = 2;
        else {
            chunk |= int(uchar(sourceData.data()[i++])) << 8;
            if (i == sourceData.size())
                padlen = 1;
            else
                chunk |= int(uchar(sourceData.data()[i++]));
        }

        int j = (chunk & 0x00fc0000) >> 18;
        int k = (chunk & 0x0003f000) >> 12;
        int l = (chunk & 0x00000fc0) >> 6;
        int m = (chunk & 0x0000003f);
        *out++ = alphabet[j];
        *out++ = alphabet[k];

        if (padlen > 1) {
            if ((options & OmitTrailingEquals) == 0)
                *out++ = padchar;
        }
        else
            *out++ = alphabet[l];
        if (padlen > 0) {
            if ((options & OmitTrailingEquals) == 0)
                *out++ = padchar;
        }
        else
            *out++ = alphabet[m];
    }

    assert((options & OmitTrailingEquals) || (out == tmp.size() + tmp.data()));
    if (options & OmitTrailingEquals)
        tmp.resize(out - tmp.data());

    return tmp;
}

std::string decode_base64(const string sourceData, int options) {
    unsigned int buf = 0;
    int nbits = 0;
    string tmp;
    tmp.resize((sourceData.size() * 3) / 4);

    int offset = 0;
    for (int i = 0; i < sourceData.size(); ++i) {
        int ch = sourceData.at(i);
        int d;

        if (ch >= 'A' && ch <= 'Z')
            d = ch - 'A';
        else if (ch >= 'a' && ch <= 'z')
            d = ch - 'a' + 26;
        else if (ch >= '0' && ch <= '9')
            d = ch - '0' + 52;
        else if (ch == '+' && (options & Base64UrlEncoding) == 0)
            d = 62;
        else if (ch == '-' && (options & Base64UrlEncoding) != 0)
            d = 62;
        else if (ch == '/' && (options & Base64UrlEncoding) == 0)
            d = 63;
        else if (ch == '_' && (options & Base64UrlEncoding) != 0)
            d = 63;
        else
            d = -1;

        if (d != -1) {
            buf = (buf << 6) | d;
            nbits += 6;
            if (nbits >= 8) {
                nbits -= 8;
                tmp[offset++] = buf >> nbits;
                buf &= (1 << nbits) - 1;
            }
        }
    }

    tmp.resize(offset);

    return tmp;
}
