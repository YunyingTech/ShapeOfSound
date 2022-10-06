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
