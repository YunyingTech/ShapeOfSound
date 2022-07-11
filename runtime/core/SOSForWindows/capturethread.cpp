/*
ģ�����ƣ����̴߳���ģ��
ʵ�ֹ��ܣ�������Ƶ����
������ߣ�������
������Ա������� ������
�����Ա������� ��Ԫ��
*/
#include "capturethread.h"

CaptureThread::CaptureThread() {}

CaptureThread::CaptureThread(queue<BufferUtil>* bufferU,int * signal) {
  this->bufferUtil = bufferU;
  this->waitSignal = signal;
}

void CaptureThread::run() {
  while (1) {
    emit threadRunning(true);
    if (stopSignal) {
      break;
      emit threadRunning(false);
    }
    BufferUtil temp;
    this->au = new AudioStream(&temp);
    au->startCapture();
    *this->waitSignal += 1;
    this->bufferUtil->push(temp);
    delete this->au;
    emit threadRunning(false);
  }
}

void CaptureThread::stopCapture(bool status) { stopSignal = status; }


CaptureThread::~CaptureThread() { delete this->au; }