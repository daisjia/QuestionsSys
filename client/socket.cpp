#include "socket.h"

Socket::Socket(const char* ip, const int port)
{
	_serfd = socket(AF_INET, SOCK_STREAM, 0);
	if (_serfd == -1)
	{
		LOGE("socket create fail!");
		return;
	}

	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	saddr.sin_addr.s_addr = inet_addr(ip);

	int ret = bind(_serfd, (struct sockaddr*) & saddr, sizeof(sockaddr));
	if (ret == -1)
	{
		LOGE("socket bind fail!");
		return;
	}
	ret = listen(_serfd, 20);
	if (ret == -1)
	{
		LOGE("server listen fail!");
		return;
	}

	_ip = ip;
	_port = port;
	LOGI("Server StartUp Success!");
}

int Socket::Connect(int fd, sockaddr_in& saddr)
{
	socklen_t len = sizeof(saddr);
	int ret = connect(fd, (struct sockaddr*)&saddr, sizeof(saddr));
	return ret;
}

int Socket::Accept(int fd, sockaddr_in& caddr)
{
	socklen_t len = sizeof(caddr);
	int clifd = accept(fd, (struct sockaddr*) & caddr, &len);
	return clifd;
}

std::string Socket::GetIp()
{
	return _ip;
}

int Socket::GetPort()
{
	return _port;
}

int Socket::GetSerFd()
{
	return _serfd;
}

