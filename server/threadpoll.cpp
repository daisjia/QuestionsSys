#include "threadpoll.h"


ThreadPoll::ThreadPoll(int threadnum)
	:_threadNum(threadnum),
	_running(true)
{
	for (int i = 0; i < _threadNum; ++i)
	{
		_thread.push_back(std::make_shared<std::thread>(std::bind(&ThreadPoll::ThreadWork, this)));
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

bool ThreadPoll::AppandTask(int fd, std::string ReqMsg)
{
	std::unique_lock<std::mutex> guard(_mtx);
	_tasks.push_back(std::pair<int, std::string>(fd, ReqMsg));
	_condition.notify_one();
	return true;
}

void ThreadPoll::ThreadWork()
{
	while (_running)
	{
		int fd;
		std::string ReqMsg;
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
				ReqMsg = _tasks.front().second;
				_tasks.pop_front();
			}
			else
			{
				continue;
			}
		} while (0);

		Task(fd, ReqMsg);
	}
}

void ThreadPoll::Task(int fd, std::string ReqMsg)
{
	__TIC1__(Task);
	PDUHEAD* pReqHeader = (PDUHEAD*)ReqMsg.data();
	_control->_model[pReqHeader->GetCmd()](fd, ReqMsg);
	__TOC1__(Task);
}
