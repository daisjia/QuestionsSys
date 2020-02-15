#pragma once
#include<vector>
#include<map>
#include<stdlib.h>
#include<memory.h>
#include<string>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<json/json.h>
#include<sys/types.h>
#include<event.h>
#include<json/json.h>
#include<iostream>
#include <fcntl.h>
#include<unistd.h>
#include "log.h"
#include<memory>

class Socket
{
public:
	Socket(const char* ip, const int port);
	~Socket() {}
	int Connect(int fd, struct sockaddr_in& saddr);
	int Accept(int fd, sockaddr_in& caddr);
	std::string GetIp();
	int GetPort();
	int GetSerFd();

private:
	std::string _ip;
	int _port;
	int _serfd;
};