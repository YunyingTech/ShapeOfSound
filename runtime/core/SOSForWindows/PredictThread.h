/*
模块名称：字幕计算模块
实现功能：多线程调度，处理结果
完成作者：樊泽瑞
测试人员：郭明皓
审核人员：郭明皓，陈嘉晖
*/
#ifndef PREDICTTHREAD_H
#define PREDICTTHREAD_H
#pragma once

//#undef slots
//#undef signals
////#include "decoder/params.h"
//#include "decoder/ctc_endpoint.h"
//#include "frontend/feature_pipeline.h"
//#include "utils/flags.h"
//#include "utils/string.h"
//#include "utils/timer.h"
//#include "utils/utils.h"
//#define slots Q_SLOTS
//#define signals Q_SIGNALS


#pragma once
#include "frontend/wav.h"
#include "api/wenet_api.h"

#include <QThread>
#include "audiostream.h"
#include "BufferUtil.h"
#include <queue>


class PredictThread : public QThread {
  Q_OBJECT
  void run() override;

 signals:
  void resultReady(const QString& s);
  void PthreadRunning(bool status);
  void sendResult(const char*);
 private slots:
  
 private:
  string get_date_time();

 public:
  queue<BufferUtil>* bufferUtil;
  PredictThread();
  PredictThread(queue<BufferUtil> * bu,int *);
  void* decoder;
  wenet::WavReader* reader;
  int* waitSignal;
};
#endif
