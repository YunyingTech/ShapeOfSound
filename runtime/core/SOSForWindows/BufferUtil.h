/*
ģ�����ƣ����洦��ģ��
ʵ�ֹ��ܣ�����鴦����Ƶ��Ϣ
������ߣ�������
������Ա������� ������
�����Ա������𩣬��Ԫ��
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
