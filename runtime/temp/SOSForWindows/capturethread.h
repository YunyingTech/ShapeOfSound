#ifndef CAPTURETHRAED_H
#define CAPTURETHRAED_H
#include <QThread>
#include "audiostream.h"

class CaptureThread : public QThread {
  Q_OBJECT
  void run() override;

signals :
  void resultReady(const QString& s);
  void threadRunning(bool status);

 public:
  std::string wave_path;
  CaptureThread();
  CaptureThread(std::string);
};

#endif  // CAPTURETHRAED_H
