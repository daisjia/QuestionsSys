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
	_control.reset(new Control);
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

bool ThreadPoll::AppandTask(int task, std::string message)
{
	std::unique_lock<std::mutex> guard(_mtx);
	_tasks.push_back(std::pair<int, std::string>(task, message));
	_condition.notify_one();
	return true;
}

void ThreadPoll::ThreadWork()
{
	while (_running)
	{
		int fd;
		std::string message;
		do
		{
			std::unique_lock<std::mutex> lck(_mtx);
			if (_tasks.empty())
			{
				_condition.wait(lck);

			}
			if (!_tasks.empty())
			{
				fd = _tasks.front().first;
				message = _tasks.front().second;
				_tasks.pop_front();
			}
			else
			{
				continue;
			}
		} while (0);

		Task(fd, message);
	}
}

void ThreadPoll::Task(int task, std::string message)
{
	_control->_model[REGISTER](task, message.c_str());
}
