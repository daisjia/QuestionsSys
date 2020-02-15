
#include "sys.h"

#define SIZE 10000
Sys* Sys::sys = NULL;
std::mutex Sys::mux;

void Sys::Recv(int fd, char* buff)
{
	while (true)
	{
		int n = recv(fd, buff, 1024 * 1024, 0);
		if (n <= 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				break;
			}
			epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd, NULL);
			_cliInfo.erase(fd);
			close(fd);
			
		}
	}
}

void SetNonBlock(int fd)
{
	int old = fcntl(fd, F_GETFL);
	int _new = old | O_NONBLOCK;
	fcntl(fd, F_SETFL, _new);
}

Sys::Sys(std::string ip, int port)
{
	_server.reset(new Socket(ip.c_str(), port));
	_threadPoll.reset(new ThreadPoll(5));
	Redis::GetRedis();
	Mysql::GetMysql();
	_epollfd = epoll_create(20);
	if (_epollfd == -1)
	{
		LOG("epollfd create fail!");
		return;
	}
	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = _server->GetSerFd();
	epoll_ctl(_epollfd, EPOLL_CTL_ADD, _server->GetSerFd(), &event);
}

void Sys::Run()
{
	while (true)
	{
		struct epoll_event events[SIZE];
		int n = epoll_wait(_epollfd, events, SIZE, -1);
		if (n <= 0)
		{
			LOG("epoll wait fail!");
			continue;
		}
		for (int i = 0; i < n; ++i)
		{
			int fd = events[i].data.fd;
			if (fd == _server->GetSerFd())
			{
				struct sockaddr_in caddr;
				int clifd = _server->Accept(_server->GetSerFd(),caddr);
				char buff[100] = { 0 };
				sprintf(buff, "ip: %s, port: %d connect success!", inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port));
				LOG(buff);
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
				char buff[100];
				sprintf(buff, "ip: %s, port: %d exit!", inet_ntoa(_cliInfo[fd].sin_addr), ntohs(_cliInfo[fd].sin_port));
				_cliInfo.erase(fd);
				LOG(buff);
			}
			else if (events[i].events & EPOLLIN)
			{
				char buff[1024 * 1024] = { 0 };
				Recv(fd, buff);
				_threadPoll->AppandTask(fd, buff);
			}
		}
	}
}