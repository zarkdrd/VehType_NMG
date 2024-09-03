/*************************************************************************
	> File Name: src/TcpClient.cpp
	> Author: ARTC
	> Descripttion:
	> Created Time: 2023-11-21
 ************************************************************************/

#include "TcpClient.h"
#include "Log_Message.h"

TcpClient::TcpClient(string IP, unsigned short port)
{
	sockfd = -1;
	ServerIP = IP;
	_port = port;
	isOpen = false;
	ConnectLen = 0;
	ThreadStatus = false;
}
TcpClient::~TcpClient()
{
	Close();
}
bool TcpClient::Open()
{
	if (ThreadStatus == false)
	{
		pthread_create(&sockID, NULL, ConnectThread, this);
		ThreadStatus = true;
	}
	return true;
}

bool TcpClient::Connect(void)
{
	if (isOpen == true)
	{
		return true;
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		log_message(ERROR, "socket: %s", strerror(errno));
		return false;
	}
	memset(&server, 0, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(_port);
	server.sin_addr.s_addr = inet_addr(ServerIP.c_str());
	log_message(INFO, "Tcp connect [%s : %d] ...", ServerIP.c_str(), _port);
	if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		if ((ConnectLen <= 0) || (ConnectLen > 600))
		{
			log_message(ERROR, "Connect [%s : %d] err: %s", ServerIP.c_str(), _port, strerror(errno));
			ConnectLen = 0;
		}
		ConnectLen++;
		close(sockfd);
		return false;
	}
	isOpen = true;
	ConnectLen = 0;
	log_message(INFO, "Connect [%s : %d] successful", ServerIP.c_str(), _port);
	return false;
}

bool TcpClient::Close()
{
	pthread_cancel(sockID);
	ThreadStatus = false;
	if (isOpen == true)
	{
		close(sockfd);
	}
	isOpen = false;
	return true;
}

bool TcpClient::Write(void *buffer, int len)
{
	if (!isOpen)
	{
		return false;
	}
	int num = send(sockfd, buffer, len, MSG_NOSIGNAL);
	if (num < 0)
	{
		log_message(ERROR, "The TcpServer disconnect: %s", strerror(errno));
		close(sockfd);
		isOpen = false;
		return -1;
	}
	return true;
}

int TcpClient::Read(void *buffer, int len)
{
	if (!isOpen)
	{
		return -1;
	}
	int RecvLen = recv(sockfd, buffer, len, 0);
	if (RecvLen <= 0)
	{
		log_message(ERROR, "The TcpServer disconnect: %s", strerror(errno));
		close(sockfd);
		isOpen = false;
		return -1;
	}

	return RecvLen;
}

int TcpClient::Read(void *buffer, int len, int sec, int msec)
{
	if (!isOpen)
	{
		log_message(ERROR, "无客户端连接");
		return -1;
	}
	int recvLen;
	fd_set rfds;
	struct timeval tv;
	tv.tv_sec = sec;
	tv.tv_usec = msec * 1000;

	FD_ZERO(&rfds);
	FD_SET(sockfd, &rfds);
	int retval = select(sockfd + 1, &rfds, NULL, NULL, &tv);
	if (retval < 0)
	{
		log_message(ERROR, "select : %s", strerror(errno));
		return -1;
	}
	else if (retval == 0)
	{
		return 0;
	}
	recvLen = recv(sockfd, buffer, len, 0);
	if (recvLen <= 0)
	{
		log_message(ERROR, "The TcpServer disconnect: %s", strerror(errno));
		close(sockfd);
		isOpen = false;
		return -1;
	}
	return recvLen;
}

bool TcpClient::Clear()
{
	return true;
}
bool TcpClient::GetStatus()
{
	return isOpen;
}

void *ConnectThread(void *arg)
{
	struct tcp_info info;
	int len = sizeof(info);
	class TcpClient *p = (class TcpClient *)arg;
	while (1)
	{
		if (p->isOpen == false)
		{
			if (p->Connect() == false)
			{
				sleep(1);
				continue;
			}
		}

		getsockopt(p->sockfd, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len);
		if ((info.tcpi_state == TCP_ESTABLISHED))
		{
			sleep(1);
		}
		else
		{
			close(p->sockfd);
			p->isOpen = false;
		}
	}

	return NULL;
}
