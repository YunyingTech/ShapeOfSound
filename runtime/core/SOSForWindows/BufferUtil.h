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
