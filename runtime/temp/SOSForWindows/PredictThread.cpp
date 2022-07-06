#include "PredictThread.h"

PredictThread::PredictThread() {  
	this->reader = new wenet::WavReader("cache.wav"); }

PredictThread::PredictThread(std::string path) {
  this->reader = new wenet::WavReader(path);
}

void PredictThread::run() {
  emit PthreadRunning(true);
  wenet_set_language(this->decoder, "chs");
  const int sample_rate = 16000;
  // Only support 16K
  const int num_samples = this->reader->num_samples();
  // Send data every 0.5 second
  const float interval = 0.5;
  const int sample_interval = interval * sample_rate;

  emit PthreadRunning(false);
}


std::string PredictThread::get_date_time() {
  auto to_string =
      [](const std::chrono::system_clock::time_point& t) -> std::string {
    auto as_time_t = std::chrono::system_clock::to_time_t(t);
    struct tm tm;
#if defined(WIN32) || defined(_WINDLL)
    localtime_s(&tm,
                &as_time_t);  // win api，线程安全，而std::localtime线程不安全
#else
    localtime_r(&as_time_t, &tm);  // linux api，线程安全
#endif

    std::chrono::milliseconds ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            t.time_since_epoch());
    char buf[128];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d\t",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour,
             tm.tm_min, tm.tm_sec, ms.count() % 1000);
    return buf;
  };

  std::chrono::system_clock::time_point t = std::chrono::system_clock::now();
  return to_string(t);
}