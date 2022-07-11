/*
模块名称：字幕显示模块
实现功能：功能的开关
完成作者：樊泽瑞
测试人员：郭明皓
审核人员：郭明皓，刘元庆
*/
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtWidgets>
#include <QWidget>
#include <string>
#include <iostream>
#include<QmessageBox>
#include "SetUp.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
  QIcon logo(":/SOSForWindows/aboutLogo.png");
  this->setWindowIcon(logo);
    this->installEventFilter(this);
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    this->txtNum = 0;
    ui->label->setAlignment(Qt::AlignCenter);
    QFont ft;
    ft.setPointSize(12);
    ui->label->setFont(ft);
    QPoint mousepos = this->pos();
    if(mousepos.rx()>=675)
    {
        this->setCursor(Qt::SizeAllCursor);
    }
    else{
        this->setCursor(Qt::ArrowCursor);
    }
    this->decoder = nullptr;
    this->loaded_model = false;
    this->model_path = "";
    this->waitSignal = 0;
    this->predicting = false;
    ui->stopRecBtn->setVisible(false);
    Qt::WindowFlags m_flags = windowFlags();
    setWindowFlags(m_flags | Qt::WindowStaysOnTopHint);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(this->rect(), QColor(0, 0, 0, 0x20)); /* 设置透明颜色 */
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_startPoint = frameGeometry().topLeft() - event->globalPos();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    this->move(event->globalPos() + m_startPoint);
}

void MainWindow::on_pushButton_clicked()
{
  captureThread = new CaptureThread(&this->bufferUtil, &this->waitSignal);
  connect(this, SIGNAL(sendStopCapture(bool)), this->captureThread,
          SLOT(CaptureThread::stopCapture(bool)));
  emit sendStopCapture(false);
  this->captureThread->wave_path = this->cache_path.c_str();
  if (loaded_model) {
    connect(captureThread, SIGNAL(threadRunning(bool)), this,
            SLOT(record_wave(bool)));

    this->captureThread->start();
  } else {
    QMessageBox::information(NULL, "Load Model", "请先加载模型");
    this->model_path =
        QFileDialog::getExistingDirectory(NULL, "获取模型目录", "./model")
            .toStdString();
    if (this->model_path != "") {
      this->decoder = wenet_init(this->model_path.c_str());
      std::chrono::system_clock::time_point t =
          std::chrono::system_clock::now();
      QMessageBox::information(NULL,
                               "Load OK",
          (get_date_time() + "模型加载成功\n").c_str());
      this->loaded_model = true;

    }
    return;
  }
    ui->label->setAlignment(Qt::AlignCenter);
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
  if (obj == this && event->type() == QEvent::WindowDeactivate) {
   
  }
  return false;
}


void MainWindow::on_stopRecBtn_clicked() {
  this->predicting = false;
  emit sendStopCapture(true);
  try {
    if (this->captureThread->isFinished()) {
      this->captureThread->destroyed();
    }
    ui->pushButton->setDisabled(false);
    ui->pushButton->setVisible(true);
    ui->stopRecBtn->setVisible(false);
  } catch (QException & e) {
    qDebug() << "OK";
  }
}


void MainWindow::record_wave(bool status) {
  if (status) {
    ui->pushButton->setDisabled(true);
    ui->pushButton->setVisible(false);
    ui->stopRecBtn->setVisible(true);
  } else {
    this->predict = new PredictThread(&this->bufferUtil, &this->waitSignal);
    connect(predict, SIGNAL(PthreadRunning(bool)), this,
            SLOT(predict_slot(bool)));
    connect(predict, SIGNAL(sendResult(const char*)), this,
            SLOT(recvResult(const char*)));
    this->predict->decoder = this->decoder;
    // wenet 识别
    if (!this->predicting) {
      this->predict->start();
    }
  }
}

void MainWindow::recvResult(const char* ret) {
  if (strlen(ret) > 0) {
    qDebug() << ret;
    QString retQString = QString::fromStdString(std::string(ret));
    QByteArray retQByte(retQString.toStdString().c_str());
    QJsonDocument retQJson = QJsonDocument::fromJson(retQByte);
    string retStdString =
        retQJson["nbest"][0]["sentence"].toString().toStdString();
    if (retStdString.length() > 90 && retStdString.length() <= 1024) {
      ui->label->setText(
          retStdString
              .substr(max((unsigned long long)0, retStdString.length() - 91),
                      max(retStdString.length() - 1, (unsigned long long)0))
              .c_str());
    } else if (retStdString.length() > 1024) {
      ui->label->setText(
          retStdString
              .substr(max((unsigned long long)0, retStdString.length() - 91),
                      max(retStdString.length() - 1, (unsigned long long)0))
              .c_str());
      wenet_reset(this->decoder);
    }
    else {
      ui->label->setText(retStdString.c_str());
    }
  }
}

std::string MainWindow::get_date_time() {
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

void MainWindow::on_toolButton_SetUp_clicked()
{
    SetUp* parent =new SetUp();
    parent ->show();//显示界面

}

void MainWindow::on_del_clicked() { 
     exit(0);
    }

void MainWindow::predict_slot(bool status) {
  if (status) {
    this->predicting = true;
  } else {
    this->predicting = false;
  }
}

