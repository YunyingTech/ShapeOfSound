/*
模块名称：字幕计算模块
实现功能：多线程调度，处理结果
完成作者：樊泽瑞
测试人员：郭明皓
审核人员：郭明皓，陈嘉晖
*/
#include "PredictThread.h"
#include <qDebug>


PredictThread::PredictThread() {  
	//this->reader = new wenet::WavReader("cache.wav"); 

}

PredictThread::PredictThread(queue<BufferUtil> * bu,int * signal) {
  //this->reader = new wenet::WavReader(path);
  this->bufferUtil = bu;
  this->waitSignal = signal;
}

void PredictThread::run() {
  emit PthreadRunning(true);
  char* resultWord;
  bool isBegin = true;
  wenet_set_language(this->decoder, "chs");
  wenet_set_timestamp(this->decoder, 1);
  while (*this->waitSignal > 0) {
    const int sample_rate = 16000;
    // Only support 16K
    const int num_samples = this->bufferUtil->front().length;
    // Send data every 0.5 second
    const float interval = 0.5;
    const int sample_interval = interval * sample_rate;

    const int buffer_pack_size = num_samples;
    if (isBegin) {
        isBegin = false;
    } else {
        wenet_add_context(this->decoder, (const char*)resultWord);
    }

    wenet_decode(this->decoder,
        (const char*)(this->bufferUtil->front().Buffer),
        buffer_pack_size, !(*this->waitSignal > 0));
    resultWord = (char *)wenet_get_result(this->decoder);
    qDebug() << resultWord;
    emit sendResult(resultWord);
    this->bufferUtil->pop();
    *this->waitSignal -= 1;
  }
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