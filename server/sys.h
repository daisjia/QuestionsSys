#pragma once
#include<iostream>
#include"threadpoll.h"
#include "redis.h"
#include "mysql.h"
#include<sys/epoll.h>
#include<map>
#include"pdu.h"
#include"ser_socket.h"

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
	Sys(std::string ip, int port);
	std::unique_ptr<ThreadPoll> _threadPoll;
	int _epollfd;
	std::map<int, struct sockaddr_in> _cliInfo;
	static std::mutex mux;
};
