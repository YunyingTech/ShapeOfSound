/*
模块名称：登录模块
实现功能：链接服务器进行发送信息
完成作者：史家旭
测试人员：郭明皓 史家旭
审核人员：郭明皓 陈嘉晖
*/

#pragma once
#ifndef MYSOCK_H
#define MYSOCK_H

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<iostream>
#include<cstdlib>
#include<time.h>
#include<winsock2.h>
#pragma comment(lib,"ws2_32.lib")
using namespace std;

SOCKET cnt();

#endif // MYSOCK_H
