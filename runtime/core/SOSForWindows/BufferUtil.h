/*
模块名称：缓存处理模块
实现功能：缓存块处理音频信息
完成作者：樊泽瑞
测试人员：郭明皓 樊泽瑞
审核人员：郭明皓，刘元庆
*/
#pragma once
#include <string.h>

typedef unsigned char BYTE;
typedef unsigned long long size_t;
typedef BYTE BufferType;

class BufferUtil {
 private:
  size_t capacity;
 public:
  BufferUtil();
  BufferUtil(BufferType*, size_t);
  BufferType* Buffer;
  size_t length;
  void resetBuffer();

  void testMem();
};
