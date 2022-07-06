#include "capturethread.h"

CaptureThread::CaptureThread() { this->wave_path = ""; }

CaptureThread::CaptureThread(std::string path) { this->wave_path = path; }

void CaptureThread::run() {
  emit threadRunning(true);
  startCapture(this->wave_path);
  emit threadRunning(false);
}
