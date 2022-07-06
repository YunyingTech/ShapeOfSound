/*
模块名称：设置模块
实现功能：跳转页面，登录，注册
完成作者：史家旭，洪铱宁，张婉琪
测试人员：郭明皓
审核人员：郭明皓，刘元庆
*/
#ifndef SETUP_H
#define SETUP_H

#include <QMainWindow>
#include <iostream>

using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui { class SetUp; }
QT_END_NAMESPACE

class SetUp : public QMainWindow
{
    Q_OBJECT

public:
    explicit SetUp(QWidget *parent = nullptr);
    ~SetUp();

private slots:


    void on_fileButton_clicked();

    void on_textButton_clicked();

    void on_newButton_clicked();

    void on_loginButton_clicked();

    void on_pushButton_6_clicked();

    void on_loginButton_3_clicked();
    void on_pushButton_2_clicked();

private:
    Ui::SetUp *ui;
};
#endif // SETUP_H
