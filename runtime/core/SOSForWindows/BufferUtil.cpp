/*
模块名称：缓存处理模块
实现功能：缓存块处理音频信息
完成作者：樊泽瑞
测试人员：郭明皓 樊泽瑞
审核人员：郭明皓，刘元庆
*/
#include "BUFFERUTIL.H"
#include <qDebug>
using namespace std;

#define CAPACITY 3000000

BufferUtil::BufferUtil() {
  capacity = CAPACITY;
  //Buffer = new BYTE[CAPACITY];
  length = 0;
}

BufferUtil::BufferUtil(BufferType* b, size_t size) {
  capacity = CAPACITY;
  //Buffer = b;
  length = size;
}

void BufferUtil::resetBuffer() { memset(this->Buffer,0, capacity); }

void BufferUtil::testMem() { 
  qDebug() << "capacity: " << this->capacity << endl;
  qDebug() << "length: " << this->length << endl;
  for (int i = 0; i < 20; i++)
    qDebug() << "Buffer[" << i << "]: " << this->Buffer[i] << endl;
}