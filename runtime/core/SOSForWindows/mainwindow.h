/*
模块名称：核心功能测试模块
实现功能：测试核心功能
完成作者：樊泽瑞
测试人员：郭明皓 樊泽瑞
审核人员：郭明皓 
*/
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
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private slots:
    void on_pushButton_clicked();
    void record_wave(bool);
    void predict_slot(bool);
    void recvResult(const char*);
    void on_toolButton_SetUp_clicked();
    void on_del_clicked();

   private:
    Ui::MainWindow *ui;
    QPoint m_startPoint;
    int txtNum;
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



#endif // MAINWINDOW_H
