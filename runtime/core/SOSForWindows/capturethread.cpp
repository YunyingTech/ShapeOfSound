/*
模块名称：多线程处理模块
实现功能：处理音频数据
完成作者：樊泽瑞
测试人员：郭明皓 樊泽瑞
审核人员：郭明皓 刘元庆
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