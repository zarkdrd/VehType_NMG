/*************************************************************************
    > File Name: inc/TcpClient.h
    > Author: ARTC
    > Descripttion:
    > Created Time: 2023-11-25
 ************************************************************************/

#ifndef _INC_TCPCLIENT_H_
#define _INC_TCPCLIENT_H_

#include <iostream>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

using namespace std;

class TcpClient
{
public:
    TcpClient(string IP, unsigned short port);
    ~TcpClient();
    bool Connect();
    bool Open();
    bool Write(void *buffer, int len);
    int Read(void *buffer, int len);
    int Read(void *buffer, int len, int sec, int msec);
    bool Close();
    bool Clear();
    bool GetStatus();

public:
    int sockfd;
    volatile bool isOpen;
    volatile bool ThreadStatus;

private:
    string ServerIP;
    unsigned short _port;
    struct sockaddr_in server;

    int ConnectLen;
    pthread_t sockID;
};

void *ConnectThread(void *arg);

#endif //_INC_TCPCLIENT_H_
