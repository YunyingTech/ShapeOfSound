#pragma once

#include <QMessageBox>
#include <QtWidgets/QMainWindow>
#include "audiostream.h"
#include "capturethread.h"
#include "ui_sosforwindows.h"
#include <QFileDialog>
#include "api/wenet_api.h"
#include "frontend/wav.h"
#include "PredictThread.h"

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

private:
  Ui::SOSForWindowsClass ui;
  CaptureThread* captureThread;
  string model_path;
  void* decoder;
  std::string get_date_time();
  bool loaded_model;
  string cache_path;
  PredictThread* predict;
};
