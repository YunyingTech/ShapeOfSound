/*
模块名称：登录模块
实现功能：链接服务器进行发送信息
完成作者：史家旭
测试人员：郭明皓 史家旭
审核人员：郭明皓 陈嘉晖
*/
#include "MySock.h"

SOCKET cnt() {
    WSADATA wsd; 
    WSAStartup(MAKEWORD(2, 2), &wsd);
    SOCKET m_SockClient; 
    sockaddr_in clientaddr;
    clientaddr.sin_family = AF_INET;
    clientaddr.sin_port = htons(4600);
    clientaddr.sin_addr.S_un.S_addr = inet_addr("118.25.1.8");
    m_SockClient = socket(AF_INET, SOCK_STREAM, 0);int i = connect(m_SockClient, (sockaddr*)&clientaddr, sizeof(clientaddr));

    return m_SockClient;
}
