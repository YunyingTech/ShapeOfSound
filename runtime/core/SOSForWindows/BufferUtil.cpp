/*
ģ�����ƣ����洦��ģ��
ʵ�ֹ��ܣ�����鴦����Ƶ��Ϣ
������ߣ�������
������Ա������� ������
�����Ա������𩣬��Ԫ��
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