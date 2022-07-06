
/*
模块名称：字幕计算模块
实现功能：录制音频，多线程调度
完成作者：樊泽瑞
测试人员：郭明皓
审核人员：郭明皓，陈嘉晖
*/

#include "sosforwindows.h"

SOSForWindows::SOSForWindows(QWidget* parent) : QMainWindow(parent) {
  ui.setupUi(this);
  this->decoder = nullptr;
  this->loaded_model = false;
  this->model_path = "";
  this->waitSignal = 0;
  this->predicting = false;
}

void SOSForWindows::on_pushButton_clicked() {
  captureThread = new CaptureThread(&this->bufferUtil,&this->waitSignal);
  this->captureThread->wave_path = this->cache_path.c_str();
  if (loaded_model) {
    connect(captureThread, SIGNAL(threadRunning(bool)), this,
            SLOT(record_wave(bool)));
   
    this->captureThread->start();
  } else {
    ui.textEdit->setTextColor(Qt::red);
    ui.textEdit->insertPlainText((get_date_time() + "请先加载模型\n").c_str());
    ui.textEdit->setTextColor(Qt::black);
    return;
  }
}

void SOSForWindows::record_wave(bool status) {
    if (status) {
        ui.pushButton->setDisabled(true);
        ui.label->setText("Running");
        ui.textEdit->insertPlainText(
            (get_date_time() + "正在录制系统声音\n").c_str());
    } else {
        ui.pushButton->setDisabled(false);
        ui.label->setText("Waiting");
        ui.textEdit->insertPlainText((get_date_time() + "录制完成\n").c_str());
        this->predict = new PredictThread(&this->bufferUtil, &this->waitSignal);
        connect(predict, SIGNAL(PthreadRunning(bool)), this,
                SLOT(predict_slot(bool)));
        connect(predict, SIGNAL(sendResult(const char*)), this,
                SLOT(recvResult(const char*)));
        this->predict->decoder = this->decoder;
        //wenet 识别
        if (!this->predicting) {
          this->predict->start();
        }
        
    }
}

void SOSForWindows::recvResult(const char* ret) {
  if (strlen(ret) > 0) {
    qDebug() << ret;
    QString retQString = QString::fromStdString(std::string(ret));
    QByteArray retQByte(retQString.toStdString().c_str());
    QJsonDocument retQJson = QJsonDocument::fromJson(retQByte);
    ui.textEdit->insertPlainText(
        (get_date_time() +
         retQJson["nbest"][0]["sentence"].toString().toStdString() + "\n")
            .c_str());
  }
}


void SOSForWindows::predict_slot(bool status) {
  if (status) {
    ui.textEdit->insertPlainText((get_date_time() + "正在识别\n").c_str());
    this->predicting = true;
  } else {
    ui.textEdit->insertPlainText((get_date_time() + "识别结束\n").c_str());
    this->predicting = false;
  }
}

std::string SOSForWindows::get_date_time() {
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

void SOSForWindows::on_selectButton_clicked()
{
    this->model_path = QFileDialog::getExistingDirectory(NULL,"获取模型目录","./model").toStdString();
  if (this->model_path != "") {
      ui.lineEdit->setText(this->model_path.c_str());
      this->decoder = wenet_init(ui.lineEdit->text().toStdString().c_str());
      std::chrono::system_clock::time_point t =
          std::chrono::system_clock::now();
      ui.textEdit->insertPlainText(
          (get_date_time() + "模型加载成功\n").c_str());
      this->loaded_model = true;
  }
}

void SOSForWindows::on_pushButton_2_clicked() {
  this->cache_path =
      QFileDialog::getExistingDirectory(NULL, "设置缓存音频文件目录", "./")
          .toStdString();
  ui.lineEdit_2->setText(this->cache_path.c_str());
  ui.textEdit->insertPlainText(
      (get_date_time() + "设置当前录制文件路径为：" + this->cache_path + "/cache.wav\n")
          .c_str());
}