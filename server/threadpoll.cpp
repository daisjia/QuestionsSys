#include "threadpoll.h"

ThreadPoll::ThreadPoll(int threadnum)
	:_threadNum(threadnum),
	_running(true)
{
	for (int i = 0; i < _threadNum; ++i)
	{
		_thread.push_back(std::make_shared<std::thread>(
			std::bind(&ThreadPoll::ThreadWork, this)));
	}
}

ThreadPoll::~ThreadPoll()
{
	if (_running)
	{
		_running = false;
		_condition.notify_all();
		for (auto t : _thread)
		{
			t->join();
		}
	}
}

bool ThreadPoll::AppandTask(int task)
{
	std::unique_lock<std::mutex> guard(_mtx);
	_tasks.push_back(task);
	_condition.notify_one();
	return true;
}

void ThreadPoll::ThreadWork()
{
	while (_running)
	{
		int task;
		{
			std::unique_lock<std::mutex> lck(_mtx);
			if (_tasks.empty())
			{
				_condition.wait(lck);
			}
			if (!_tasks.empty())
			{
				task = _tasks.front();
				_tasks.pop_front();
			}
			else
			{
				continue;
			}
		}
		Task(task);
	}
}

void ThreadPoll::Task(int task)
{
}
