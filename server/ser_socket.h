#pragma once

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <poll.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string>
#include <vector>
#include<mutex>
#include <map>
#include"pdu.h"



#define CHECK_HOSP() \
	if(_hostIp.size() <= 0 || _hostPort == 0) \
	{ \
		snprintf(_errStr, 256, "[%s][%d] invalid ip or port\n", \
								__FILE__, __LINE__); \
		std::cout<<_errStr<<std::endl; \
		return -1; \
	}

#define CHECK_SOCK() \
	if(_sockfd <= 0) \
	{ \
		snprintf(_errStr, 256, "[%s][%d] invalid sockfd\n", \
								__FILE__, __LINE__); \
		std::cout<<_errStr<<std::endl; \
		return -1; \
	}

#define CHECK_INPUT() \
	if(_func == 0) \
	{ \
		snprintf(_errStr, 256, "[%s][%d] invalid Protocol fun\n", \
								__FILE__, __LINE__); \
		std::cout<<_errStr<<std::endl; \
		return -1; \
	}

#define CHECK_ALL() \
	CHECK_SOCK(); \
	CHECK_HOSP(); \
	CHECK_INPUT(); \

typedef int (*FUNC_CLIENT_INPUT)(const char* buff, int len);
int PduProtocol(const char* buff, int len);
enum ENUM_SOCK_RSP
{
	SOCK_SUC = 0x00,
	ERROR_RECV = -20,
	ERROR_SEND,
	ERROR_ACCEPT,
	ERROR_NONBLOCK
};

class SerSocket
{
public:
	
	~SerSocket();
	static SerSocket* GetSerSocket()
	{
		if (ser == NULL)
		{
			lck.lock();
			if (ser == NULL)
			{
				ser = new SerSocket();
			}
			lck.unlock();
		}
		return ser;
	}

	int Init(const char* ip, int port);
	int Close();
	void GetHost(std::string& ip, int& port);
	int Accept(sockaddr_in &caddr);
	int SendBuf(int fd, char * messages, int len);
	int RecvBuf(int fd, char * message, int needlen);
	void SetPduProtocol();
	char* GetErrMsg() { return (char*)&_errStr[0]; };
	int GetSockFd() { return _sockfd; }
	static SerSocket* ser;
private:
	int CheckRead(int fd);
	int CheckWrite(int fd);
	int CheckReadAndWrite(int fd);
	int SetNonBlock(int fd);
private:
	char _errStr[512];
	int _sockfd;
	FUNC_CLIENT_INPUT _func;
	std::string _hostIp;
	int _hostPort;
	struct sockaddr_in _hostAddr;
	socklen_t _hostLen;
	
	static std::mutex lck;
	SerSocket();
};