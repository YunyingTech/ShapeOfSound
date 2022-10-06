
#pragma once
#ifndef SETUP_H
#define SETUP_H

#include <QMainWindow>
#include <iostream>
#include "ui_SetUp.h"
#include "base64.h"
#include "rsa_a.h"
#include "MySock.h"
#include <qicon.h>
#include <QFileDialog>
#include <string.h>
#include "mainwindow.h"
#include <QMainWindow>

#include<string>


using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui { class SetUp; }
QT_END_NAMESPACE

class MainWindow;

class SetUp : public QMainWindow
{
    Q_OBJECT

public:
    explicit SetUp(QWidget *parent = nullptr);
    ~SetUp();

private slots:

    void on_modelBrowser_clicked();
    void on_fileButton_clicked();

    void on_textButton_clicked();

    void on_newButton_clicked();

    void on_loginButton_clicked();

    void on_pushButton_6_clicked();
    void on_textFontBtn_clicked();
    void on_loginButton_3_clicked();
    void on_pushButton_2_clicked();
    void on_modelPathCheckBtn_clicked();
    
signals:
    void setLabelRGB(int r, int g, int b);

private:
 Ui::SetUp *ui;
 void* mainPtr;
};

#endif // SETUP_H
