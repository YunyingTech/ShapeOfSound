/*
模块名称：界面显示模块
实现功能：界面显示逻辑
完成作者：张婉琪 郭明皓
测试人员：郭明皓
审核人员：刘元庆 陈嘉晖
*/
#include "capturethread.h"
#include "sosforwindows.h"
#include "mainwindow.h"
#include <QtWidgets/QApplication>
#include <iostream>
#include <QDebug>
#include <thread>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
