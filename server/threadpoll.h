#pragma once
#include<thread>
#include<mutex>
#include<condition_variable>
#include<list>
#include<vector>
#include<memory>
#include<functional>
#include<iostream>

class ThreadPoll
{
public:
	ThreadPoll(int threadnum);
	~ThreadPoll();
	bool AppandTask(int task);
	void ThreadWork(void);
	void Task(int task);

public:
	std::mutex _mtx;
	std::condition_variable _condition;
	bool _running;
	int _threadNum;
	std::vector<std::shared_ptr<std::thread>> _thread;
	std::list<int> _tasks;
};