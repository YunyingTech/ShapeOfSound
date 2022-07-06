/*
ģ�����ƣ����̴߳���ģ��
ʵ�ֹ��ܣ�������Ƶ����
������ߣ�������
������Ա������� ������
�����Ա������� ��Ԫ��
*/
#ifndef CAPTURETHRAED_H
#define CAPTURETHRAED_H
#include <QThread>
#include "audiostream.h"
#include "BufferUtil.h"
#include <queue>

class CaptureThread : public QThread {
  Q_OBJECT
  void run() override;

signals :
  void resultReady(const QString& s);
  void threadRunning(bool status);

 public:
  std::string wave_path;
  CaptureThread();
  CaptureThread(queue<BufferUtil>* bufferU,int *);
  ~CaptureThread();
  AudioStream * au;
  int* waitSignal;
  queue<BufferUtil>* bufferUtil;
};

#endif  // CAPTURETHRAED_H
