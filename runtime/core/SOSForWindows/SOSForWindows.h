#pragma once
/*
模块名称：字幕计算模块
实现功能：录制音频，多线程调度
完成作者：樊泽瑞
测试人员：郭明皓
审核人员：郭明皓，陈嘉晖
*/

#include <QMessageBox>
#include <QtWidgets/QMainWindow>
#include "audiostream.h"
#include "capturethread.h"
#include "ui_sosforwindows.h"
#include <QFileDialog>
#include <queue>
#include <qjsonobject.h>
#include <qjsondocument.h>
#include <qjsonvalue.h>
#include "PredictThread.h"
#include "BufferUtil.h"

class SOSForWindows : public QMainWindow {
  Q_OBJECT

 public:
  SOSForWindows(QWidget* parent = Q_NULLPTR);
 private slots:
  void on_pushButton_clicked();
  void record_wave(bool);
  void on_pushButton_2_clicked();
  void on_selectButton_clicked();
  void predict_slot(bool);
  void recvResult(const char*);

private:
  Ui::SOSForWindowsClass ui;
  CaptureThread* captureThread;
  string model_path;
  void* decoder;
  std::string get_date_time();
  bool loaded_model;
  string cache_path;
  PredictThread* predict;
  queue<BufferUtil> bufferUtil;
  int waitSignal;
  bool predicting;
};