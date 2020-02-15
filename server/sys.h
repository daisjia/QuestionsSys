#pragma once
#include<iostream>
#include"threadpoll.h"
#include "socket.h"
#include "redis.h"
#include "mysql.h"
#include<sys/epoll.h>
#include<map>
#include"pdu.h"

class Sys
{
public:
	static Sys* GetSys(std::string ip, int port)
	{
		if (sys == NULL)
		{
			mux.lock();
			if (sys == NULL)
			{
				sys = new Sys(ip, port);
			}
			mux.unlock();
		}
		return sys;
	}

	void Run();	
	static Sys* sys;
private:
	void Recv(int fd, char* buff);
	Sys(std::string ip, int port);
	std::unique_ptr<Socket> _server;
	std::unique_ptr<ThreadPoll> _threadPoll;
	int _epollfd;
	std::map<int, struct sockaddr_in> _cliInfo;
	static std::mutex mux;
};
