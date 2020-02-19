#include "sys.h"

#define SIZE 10000
Sys* Sys::sys = NULL;
std::mutex Sys::mux;

void SetNonBlock(int fd)
{
	int old = fcntl(fd, F_GETFL);
	int _new = old | O_NONBLOCK;
	fcntl(fd, F_SETFL, _new);
}

Sys::Sys(std::string ip, int port)
{
	int res = 0;
	SerSocket* ser = SerSocket::GetSerSocket();
	res = ser->Init(ip.c_str(), port);
	if (res != SOCK_SUC)
	{
		std::cout << SerSocket::GetSerSocket()->GetErrMsg() << std::endl;
		exit(0);
	}
	_threadPoll.reset(new ThreadPoll(4));
	RedisPool* redis = RedisPool::GetRedisPool();
	redis->SetConf(5);
	MysqlPool *mysql = MysqlPool::GetMysqlPool();
	mysql->SetConf("127.0.0.1", "root", "123456", "item", 4);
	_epollfd = epoll_create(20);
	if (_epollfd == -1)
	{
		LOGE("epollfd create fail!");
		exit(0);
	}
	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = ser->GetSockFd();
	epoll_ctl(_epollfd, EPOLL_CTL_ADD, ser->GetSockFd(), &event);
}

void Sys::Run()
{
	while (true)
	{
		__TIC1__(Epoll_Wait);
		struct epoll_event events[SIZE];
		int n = epoll_wait(_epollfd, events, SIZE, -1);
		if (n <= 0)
		{
			LOGE("epoll wait fail!");
			continue;
		}
		for (int i = 0; i < n; ++i)
		{
			int fd = events[i].data.fd;
			if (fd == SerSocket::GetSerSocket()->GetSockFd())
			{
				struct sockaddr_in caddr;
				int clifd = SerSocket::GetSerSocket()->Accept(caddr);
				LOGI("ip: %s, port: %d connect success!", inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port));
				if (clifd <= 0)
				{
					continue;
				}
				SetNonBlock(clifd);
				struct epoll_event event;
				event.events = EPOLLIN | EPOLLRDHUP;
				event.data.fd = clifd;
				epoll_ctl(_epollfd, EPOLL_CTL_ADD, clifd, &event);
				_cliInfo.insert(std::make_pair(clifd, caddr));
			}
			else if (events[i].events & EPOLLRDHUP)
			{
				epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd, NULL);
				close(fd);
				LOGI("ip: %s, port: %d exit!", inet_ntoa(_cliInfo[fd].sin_addr), ntohs(_cliInfo[fd].sin_port));
				_cliInfo.erase(fd);
			}
			else if (events[i].events & EPOLLIN)
			{
				char RspBuffer[1024 * 1024] = { 0 };
				int res = SerSocket::GetSerSocket()->RecvBuf(fd, RspBuffer, 1024 * 1024);
				if (res != SOCK_SUC)
				{
					std::cout << SerSocket::GetSerSocket()->GetErrMsg() << std::endl;
					continue;
				}
				PDUHEAD* pReqHeader = (PDUHEAD*)RspBuffer;
				std::string ReqMsg(RspBuffer, pReqHeader->GetPackLen());
				_threadPoll->AppandTask(fd, ReqMsg);
			}
		}
		__TOC1__(Epoll_Wait);
	}
}