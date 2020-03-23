#include "threadpoll.h"


ThreadPoll::ThreadPoll(int threadnum)
	:_threadNum(threadnum),
	_running(true)
{
	for (int i = 0; i < _threadNum; ++i)
	{
		_thread.push_back(std::make_shared<std::thread>(std::bind(&ThreadPoll::ThreadWork, this)));
		std::cout << "----------------Thread id: < "<< _thread[i]->get_id() <<" > Running...----------------" << std::endl;
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
	LOGE("bool ThreadPoll::AppandTask(int fd, std::string ReqMsg)");
	std::unique_lock<std::mutex> guard(_mtx);
	_tasks.push_back(std::pair<int, std::string>(fd, ReqMsg));
	_condition.notify_one();
	LOGE("bool ThreadPoll::AppandTask(int fd, std::string ReqMsg)-->end");
	return true;
}

void ThreadPoll::ThreadWork()
{
	LOGE("void ThreadPoll::ThreadWork()");
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
				std::cout << "----------------Thread id: < " << std::this_thread::get_id() << " > Dealing...----------------" << std::endl;
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
	LOGE("void ThreadPoll::ThreadWork()-->end");
}

void ThreadPoll::Task(int fd, std::string ReqMsg)
{
	LOGE("void ThreadPoll::Task(int fd, std::string ReqMsg)");
	__TIC1__(Task);
	PDUHEAD* pReqHeader = (PDUHEAD*)ReqMsg.data();
	auto it = _control->_model.find(pReqHeader->GetCmd());
	if (it == _control->_model.end())
	{
		LOGE("req cmd error!");
		return;
	}

	int res = it->second(fd, ReqMsg);
	if(!res)
	{
		LOGI("deal task fail!");
	}
	__TOC1__(Task);
	LOGE("void ThreadPoll::Task(int fd, std::string ReqMsg)-->end");
}
