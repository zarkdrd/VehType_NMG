/*************************************************************************
	> File Name: inc/TcpServer.h
	> Author: ARTC
	> Descripttion:
	> Created Time: 2023-10-30
 ************************************************************************/

#ifndef _INCLUDE_TCPSERVER_H_
#define _INCLUDE_TCPSERVER_H_

#include <iostream>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

using namespace std;

class TcpServer
{
public:
	TcpServer(unsigned short port);
	~TcpServer();
	bool Open();
	bool Close();
	bool WriteByte(void *buffer, int len);
	int ReadByte(void *buffer, int len, int sec, int msec);
	int RecvByte(void *buffer, int len);
	bool Accept();

public:
	bool isOpen;

private:
	int sockfd, newfd;
	pthread_t sockID;
	unsigned short _port;
};

void *AcceptThread(void *arg);

#endif //_INCLUDE_TCPSERVER_H_
