/*
模块名称：设置模块
实现功能：跳转页面，登录，注册
完成作者：史家旭，洪铱宁，张婉琪
测试人员：郭明皓
审核人员：郭明皓，刘元庆
*/


#include "SetUp.h"
#include "ui_SetUp.h"
#include "base64.h"
#include "rsa_a.h"
#include "MySock.h"

#include<string>
#include<QMessageBox>

using namespace std;
SetUp::SetUp(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SetUp)
{
    ui->setupUi(this);
}

SetUp::~SetUp()
{
    delete ui;
}


void SetUp::on_fileButton_clicked()
{ ui->stackedWidget->setCurrentIndex(0); }


void SetUp::on_textButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}



void SetUp::on_newButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
}


void SetUp::on_loginButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(3);
}


void SetUp::on_pushButton_6_clicked()
{
    ui->stackedWidget->setCurrentIndex(4);
}

void SetUp::on_loginButton_3_clicked()
{
    string userName = ui->username_2->text().toStdString();//读入输入的账号
    string password = ui->password->text().toStdString();//读入输入的密码
    QString name = QString::fromStdString(userName);

    SOCKET m_SockClient = cnt();

    char buffer[1024];
    char inBuf[1024];
    int num;
    num = recv(m_SockClient, buffer, 1024, 0);             //×èÈûº¯Êý,µÈ´ý½ÓÊÜÄÚÈÝ
    if (num > 0)                //×èÈû
    {
        // cout << "Receive form server" << buffer << endl;

        memset(inBuf, 0, 1024);
        strcpy(inBuf, "GET_KEY");
        send(m_SockClient, inBuf, sizeof inBuf, 0);

        recv(m_SockClient, buffer, 1024, 0);
        cout << buffer << endl;

        memset(inBuf, 0, 1024);
        strcpy(inBuf, "PK");
        send(m_SockClient, inBuf, sizeof inBuf, 0);
        recv(m_SockClient, buffer, 1024, 0);
        long long pk = 0;
        for (int i = 0; i < strlen(buffer); i++) {
            pk += buffer[i] - '0';
            pk *= 10;
        }
        pk /= 10;

        memset(inBuf, 0, 1024);
        strcpy(inBuf, "SK");
        send(m_SockClient, inBuf, sizeof inBuf, 0);
        recv(m_SockClient, buffer, 1024, 0);
        long long sk = 0;
        for (int i = 0; i < strlen(buffer); i++) {
            sk += buffer[i] - '0';
            sk *= 10;
        }
        sk /= 10;

        RSA rsa(sk, pk);
        Key key = rsa.GetKey();
        string strin(encode_base64(password));
        vector<long> strecrept = rsa.Ecrept(strin, key.public_key, key.share_key);

        memset(inBuf, 0, 1024);
        strcpy(inBuf, "USERNAME");
        send(m_SockClient, inBuf, sizeof inBuf, 0);
        memset(inBuf, 0, 1024);
        strcpy(inBuf, (char*)userName.c_str());
        send(m_SockClient, inBuf, sizeof inBuf, 0);

        memset(inBuf, 0, 1024);
        strcpy(inBuf, "PASSWORD");
        send(m_SockClient, inBuf, sizeof inBuf, 0);
        memset(inBuf, 0, 1024);
        sprintf(inBuf, "%d", (int)strecrept.size());
        send(m_SockClient, inBuf, sizeof inBuf, 0);

        for (int i = 0; i < strecrept.size(); i++) {
            memset(inBuf, 0, 1024);
            sprintf(inBuf, "%ld", strecrept[i]);
            send(m_SockClient, inBuf, sizeof inBuf, 0);
        }

        recv(m_SockClient, buffer, 1024, 0);
        if (buffer[0] == '0') {
            cout << "User Not Found" << endl;
          QMessageBox::warning(NULL, "错误", "用户不存在");
        }
        else if (buffer[0] == '-') {
            cout << "Username or password incorrect" << endl;
          QMessageBox::warning(NULL, "错误", "用户名或密码错误");

        }
        else {
            cout << "AC" << endl;
          QMessageBox::information(NULL, "欢迎", name);
            ui->stackedWidget->setCurrentIndex(0);
          
        }
    }

    memset(inBuf, 0, 1024);
    strcpy(inBuf, "exit");
    send(m_SockClient, inBuf, sizeof inBuf, 0);
    //用弹窗的形式把账号和密码内容显示出来
    //QMessageBox::information(NULL, userName.c_str(), password.c_str(), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
}



void SetUp::on_pushButton_2_clicked()
{
    string userName = ui->username->text().toStdString();//读入输入的账号
    string password = ui->password_2->text().toStdString();//读入输入的密码

    SOCKET m_SockClient = cnt();

    char buffer[1024];
    char inBuf[1024];
    int num;
    num = recv(m_SockClient, buffer, 1024, 0);

    memset(inBuf, 0, 1024);
    strcpy(inBuf, "REG");
    send(m_SockClient, inBuf, sizeof inBuf, 0);
    memset(inBuf, 0, 1024);
    strcpy(inBuf, "GET_KEY");
    send(m_SockClient, inBuf, sizeof inBuf, 0);

    recv(m_SockClient, buffer, 1024, 0);
    cout << buffer << endl;

    memset(inBuf, 0, 1024);
    strcpy(inBuf, "PK");
    send(m_SockClient, inBuf, sizeof inBuf, 0);
    recv(m_SockClient, buffer, 1024, 0);
    long long pk = 0;
    for (int i = 0; i < strlen(buffer); i++) {
        pk += buffer[i] - '0';
        pk *= 10;
    }
    pk /= 10;

    memset(inBuf, 0, 1024);
    strcpy(inBuf, "SK");
    send(m_SockClient, inBuf, sizeof inBuf, 0);
    recv(m_SockClient, buffer, 1024, 0);
    long long sk = 0;
    for (int i = 0; i < strlen(buffer); i++) {
        sk += buffer[i] - '0';
        sk *= 10;
    }
    sk /= 10;

    RSA rsa(sk, pk);
    Key key = rsa.GetKey();
    string strin(encode_base64(password));
    vector<long> strecrept = rsa.Ecrept(strin, key.public_key, key.share_key);

    memset(inBuf, 0, 1024);
    strcpy(inBuf, "USERNAME");
    send(m_SockClient, inBuf, sizeof inBuf, 0);
    memset(inBuf, 0, 1024);
    strcpy(inBuf, (char*)userName.c_str());
    send(m_SockClient, inBuf, sizeof inBuf, 0);

    memset(inBuf, 0, 1024);
    strcpy(inBuf, "PASSWORD");
    send(m_SockClient, inBuf, sizeof inBuf, 0);
    memset(inBuf, 0, 1024);
    sprintf(inBuf, "%d", (int)strecrept.size());
    send(m_SockClient, inBuf, sizeof inBuf, 0);

    for (int i = 0; i < strecrept.size(); i++) {
        memset(inBuf, 0, 1024);
        sprintf(inBuf, "%ld", strecrept[i]);
        send(m_SockClient, inBuf, sizeof inBuf, 0);
    }
    recv(m_SockClient, buffer, 1024, 0);

    if (buffer[0] == '1') {
        cout << "User added" << endl;
      QMessageBox::information(NULL, "注册成功", "用户已添加");
    }
    else {
        cout << "Username already exists";
      QMessageBox::warning(NULL, "注册失败", "用户名已存在");
    }

    memset(inBuf, 0, 1024);
    strcpy(inBuf, "exit");
    send(m_SockClient, inBuf, sizeof inBuf, 0);
}

