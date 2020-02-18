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
#include <map>
#include<mutex>
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
	ERROR_CONNECT,
	ERROR_NONBLOCK
};

class CliSocket
{
public:
	~CliSocket();
	static CliSocket* GetCliSocket()
	{
		if (cli == NULL)
		{
			lck.lock();
			if (cli == NULL)
			{
				cli = new CliSocket();
			}
			lck.unlock();
		}
		return cli;
	}

	int Init(const char* ip, int port, bool keep_alive = true);
	int Close();
	void GetHost(std::string& ip, int& port);
	int Connect();
	int SendBuf(char * messages, int len);
	int RecvBuf(char * message, int needlen, int& recvlen);
	void SetPduProtocol();
	char* GetErrMsg() { return (char*)&_errStr[0]; };

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
	bool _keepAlive;
	struct sockaddr_in _hostAddr;
	socklen_t _hostLen;
	static CliSocket* cli;
	static std::mutex lck;
	CliSocket();
};