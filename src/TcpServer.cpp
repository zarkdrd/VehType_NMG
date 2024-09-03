/*************************************************************************
	> File Name: src/TcpServer.cpp
	> Author: ARTC
	> Descripttion:
	> Created Time: 2024-4-16
 ************************************************************************/

#include "TcpServer.h"
#include "Log_Message.h"

TcpServer::TcpServer(unsigned short port)
{
	isOpen = false;
	_port = port;
}

TcpServer::~TcpServer()
{
	Close();
	pthread_cancel(sockID);
	close(sockfd);
}

bool TcpServer::Open()
{
	struct sockaddr_in server_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		log_message(ERROR, "socket : %s", strerror(errno));
		return false;
	}
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(_port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int keepAlive = 1;	  // 开启keepalive属性
	int keepIdle = 5;	  // 如该连接在5秒内没有任何数据往来,则进行探测
	int keepInterval = 1; // 探测时发包的时间间隔为1秒
	int keepCount = 3;	  // 探测尝试的次数.如果第1次探测包就收到响应了,则后2次的不再发.
	setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive, sizeof(keepAlive));
	setsockopt(sockfd, SOL_TCP, TCP_KEEPIDLE, (void *)&keepIdle, sizeof(keepIdle));
	setsockopt(sockfd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
	setsockopt(sockfd, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount));

	int opt = 1;
	/**绑定端口号之前，清除之前的绑定**/
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	/**禁用nagle算法，防止粘包**/
	int bNodelay = 1;
	setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&bNodelay, sizeof(bNodelay));

	if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		close(sockfd);
		log_message(ERROR, "bind : %s", strerror(errno));
		return false;
	}
	if (listen(sockfd, 5) < 0)
	{
		close(sockfd);
		log_message(ERROR, "listen : %s", strerror(errno));
		return false;
	}

	/*创建线程等待客户端连接*/
	pthread_create(&sockID, NULL, AcceptThread, this);

	log_message(INFO, "Tcp Server [%d] Init successful !", _port);

	return true;
}

bool TcpServer::Close()
{
	isOpen = false;
	shutdown(newfd, SHUT_RD);
	close(newfd);
	return true;
}

bool TcpServer::WriteByte(void *buffer, int len)
{
	if (!isOpen)
	{
		log_message(ERROR, "无客户端连接");
		return false;
	}
	int flag = 0;
	flag = send(newfd, buffer, len, MSG_NOSIGNAL);
	// printf("智能节点发送：%d字节\n",flag);
	return true;
}

int TcpServer::ReadByte(void *buffer, int len, int sec, int msec)
{
	if (!isOpen)
	{
		log_message(ERROR, "无客户端连接");
		return -1;
	}
	fd_set rfds;
	struct timeval tv;
	tv.tv_sec = sec;
	tv.tv_usec = msec * 1000;

	FD_ZERO(&rfds);
	FD_SET(newfd, &rfds);
	int retval = select(newfd + 1, &rfds, NULL, NULL, &tv);
	if (retval < 0)
	{
		log_message(ERROR, "select : %s", strerror(errno));
		return -1;
	}
	else if (retval == 0)
	{
		return 0;
	}

	return recv(newfd, buffer, len, 0);
}

int TcpServer::RecvByte(void *buffer, int len)
{
	return recv(newfd, buffer, len, 0);
}

bool TcpServer::Accept()
{
	struct sockaddr_in client_addr;
	socklen_t sin_size = sizeof(client_addr);
	int new_fd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);
	if (new_fd < 0)
	{
		log_message(ERROR, "accept : %s", strerror(errno));
		return -1;
	}

	if (isOpen)
	{
		Close();
		log_message(INFO, "Close the old connection");
		sleep(1);
	}

	log_message(INFO, "I got a new connection [%d] from (%s:%d)", new_fd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

	newfd = new_fd;
	isOpen = true;

	return new_fd;
}

void *AcceptThread(void *arg)
{
	class TcpServer *Tcp = (class TcpServer *)arg;
	log_message(INFO, "等待客户端进行连接...");
	while (1)
	{
		if (Tcp->Accept() == false)
		{
			sleep(1);
		}
	}
}
