#include "capturethread.h"
#include "sosforwindows.h"
#include <QtWidgets/QApplication>
#include <iostream>
#include <QDebug>
#include <thread>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SOSForWindows w;
    w.show();
    return a.exec();
}
