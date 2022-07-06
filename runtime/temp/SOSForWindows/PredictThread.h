#ifndef PREDICTTHREAD_H
#define PREDICTTHREAD_H
#pragma once
#include <QThread>
#include "audiostream.h"
#include "frontend/wav.h"

#include "api/wenet_api.h"

class PredictThread : public QThread {
  Q_OBJECT
  void run() override;

 signals:
  void resultReady(const QString& s);
  void PthreadRunning(bool status);
  
 private:
  string get_date_time();

 public:
  PredictThread();
  PredictThread(std::string path);
  void* decoder;
  wenet::WavReader* reader;
};
#endif
