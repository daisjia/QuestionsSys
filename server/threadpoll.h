#pragma once
#include<thread>
#include<mutex>
#include<condition_variable>
#include<list>
#include<vector>
#include<memory>
#include<functional>
#include<iostream>
#include"mytime.h"
#include"human.pb.h"
#include"control.h"
#include<stdlib.h>
#include<unistd.h>
#include<iostream>


class ThreadPoll
{
public:
	ThreadPoll(int threadnum);
	~ThreadPoll();
	bool AppandTask(int task, std::string message);
	void ThreadWork(void);
	void Task(int task, std::string message);

public:
	std::mutex _mtx;
	std::condition_variable _condition;
	bool _running;
	int _threadNum;
	std::vector<std::shared_ptr<std::thread>> _thread;
	std::list<std::pair<int, std::string>> _tasks;
	std::unique_ptr<Control> _control;
};