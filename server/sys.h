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
	static Sys* GetSys()
	{
		if (sys == NULL)
		{
			mux.lock();
			if (sys == NULL)
			{
				sys = new Sys();
			}
			mux.unlock();
		}
		return sys;
	}

	void Run();	
	static Sys* sys;

	~Sys();
private:
	Sys();
	std::unique_ptr<ThreadPoll> _threadPoll;
	//int _epollfd;
	//std::map<int, struct sockaddr_in> _cliInfo;
	
	static std::mutex mux;
	int balancefd;  //�͸��ؾ������ӵ��׽���

	std::vector<std::thread> t;
};
