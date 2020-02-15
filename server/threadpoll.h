#pragma once
#include<thread>
#include<mutex>
#include<condition_variable>
#include<list>
#include<vector>
#include<memory>
#include<functional>
#include<iostream>
#include "log.h"
#include"control.h"
#include<stdlib.h>
#include<unistd.h>
#include<iostream>
#include"pdu.h"


class ThreadPoll
{
public:
	ThreadPoll(int threadnum);
	~ThreadPoll();
	bool AppandTask(int task, std::string ReqMsg);
	void ThreadWork(void);
	void Task(int fd, std::string ReqMsg);

public:
	std::mutex _mtx;
	std::condition_variable _condition;
	bool _running;
	int _threadNum;
	std::vector<std::shared_ptr<std::thread>> _thread;
	std::list<std::pair<int, std::string>> _tasks;
	std::unique_ptr<Control> _control;
};