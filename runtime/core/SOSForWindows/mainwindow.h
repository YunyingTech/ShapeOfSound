
#pragma once
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "SetUp.h"
#include <QMessageBox>
#include <QtWidgets/QMainWindow>
#include "audiostream.h"
#include "capturethread.h"
#include <QFileDialog>
#include <queue>
#include <qjsonobject.h>
#include <qjsondocument.h>
#include <qjsonvalue.h>
#include "PredictThread.h"
#include "BufferUtil.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}  // namespace Ui
QT_END_NAMESPACE


class SetUp;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    bool eventFilter(QObject* obj, QEvent* event);

signals:
    void sendStopCapture(bool status);

private slots:
    void on_pushButton_clicked();
    void record_wave(bool);
    void predict_slot(bool);
    void recvResult(const char*);
    void on_toolButton_SetUp_clicked();
    void on_stopRecBtn_clicked();
    void on_del_clicked();

public slots:
    void setLabelRGBSlot(int, int, int);

private:
    QPoint m_startPoint;
    int txtNum;
    CaptureThread* captureThread;
    std::string get_date_time();
    string cache_path;
    PredictThread* predict;
    queue<BufferUtil> bufferUtil;
    int waitSignal;
    bool predicting;
    Ui::MainWindow* ui;
    SetUp* setUpUi;


 public:
    string model_path;
    void* decoder;
    bool loaded_model;

};


#endif // MAINWINDOW_H
