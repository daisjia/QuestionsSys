#include "sys.h"

#define SIZE 10000
Sys* Sys::sys = NULL;
std::mutex Sys::mux;
std::vector<Redis*> redisRead;
Redis* redisWrite;

int _epollfd;
std::map<int, struct sockaddr_in> _cliInfo;

void SetNonBlock(int fd)
{
	int old = fcntl(fd, F_GETFL);
	int _new = old | O_NONBLOCK;
	fcntl(fd, F_SETFL, _new);
}

void HeartRate(int fd)
{
	while (true)
	{
		char Buffer[5] = { 0 };
		recv(fd, (void*)Buffer, 5, 0);
		send(fd, (void*)"PONG", 5, 0);
	}
}

Sys::~Sys()
{
	_threadPoll.reset();
	for (auto& it : t)
		it.join();

	for (auto it : redisRead)
		delete it;

	redisRead.clear();
	delete redisWrite;
	redisWrite = nullptr;
	_cliInfo.clear();
}

Sys::Sys()
{
	int res = 0;
	sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(5000); //负载均衡服务器套接字为5000
	saddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	balancefd = socket(AF_INET, SOCK_STREAM, 0);
	if (balancefd <= 0)
	{
		LOGI("server start fail, balancefd <= 0 !");
		exit(0);
	}
	socklen_t len;
	res = connect(balancefd, (struct sockaddr*) & saddr, sizeof(saddr));
	if (res == -1)
	{
		LOGI("server start fail, connect fail!");
		exit(0);
	}

	int port = 0;
	res = recv(balancefd, (void*)&port, 4, 0);
	if (res <= 0)
	{
		LOGI("server start fail, recv <= 0!");
		exit(0);
	}

	if (port < 5002)
	{
		LOGI("server start fail, port < 5002!");
		exit(0);
	}

	SerSocket* ser = SerSocket::GetSerSocket();
	res = ser->Init("127.0.0.1", port);
	if (res != SOCK_SUC)
	{
		std::cout << SerSocket::GetSerSocket()->GetErrMsg() << std::endl;
		exit(0);
	}
	_threadPoll.reset(new ThreadPoll(5));

	std::cout << std::endl;
	MysqlPool* mysql = MysqlPool::GetMysqlPool();
	mysql->SetConf("127.0.0.1", "root", "123456", "item", 5);

	std::cout << std::endl;
	/*RedisPool* redis = RedisPool::GetRedisPool();
	redis->SetConf(5);*/
	redisWrite = new Redis("127.0.0.1", 6379);
	redisRead.push_back(new Redis("127.0.0.1", 6380));
	redisRead.push_back(new Redis("127.0.0.1", 6381));
	redisRead.push_back(new Redis("127.0.0.1", 6380));
	redisRead.push_back(new Redis("127.0.0.1", 6381));
	redisRead.push_back(new Redis("127.0.0.1", 6380));
	redisRead.push_back(new Redis("127.0.0.1", 6381));

	for (auto it : redisRead)
	{
		it->ExeCmd("slaveof 127.0.0.1 6379");
	}

	_epollfd = epoll_create(20);
	if (_epollfd == -1)
	{
		LOGE("epollfd create fail!");
		exit(0);
	}
	struct epoll_event event, event1;
	event.events = EPOLLIN;
	event.data.fd = ser->GetSockFd();
	epoll_ctl(_epollfd, EPOLL_CTL_ADD, ser->GetSockFd(), &event);

	t.push_back(std::thread(HeartRate, balancefd));
}

void Sys::Run()
{
	while (true)
	{
		//std::cout << "===================================" << std::endl;
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
				LOGI("client ip: %s, port: %d connect success!", inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port));
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
				LOGI("client ip: %s, port: %d exit!", inet_ntoa(_cliInfo[fd].sin_addr), ntohs(_cliInfo[fd].sin_port));
				_cliInfo.erase(fd);
			}
			else if (events[i].events & EPOLLIN)
			{
				char RspBuffer[1024 * 1024] = { 0 };
				int res = SerSocket::GetSerSocket()->RecvBuf(fd, RspBuffer, 1024 * 1024);
				if (res != SOCK_SUC)
				{
					epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd, NULL);
					close(fd);
					//LOGI("ip: %s, port: %d exit!", inet_ntoa(_cliInfo[fd].sin_addr), ntohs(_cliInfo[fd].sin_port));
					_cliInfo.erase(fd);
					//std::cout << SerSocket::GetSerSocket()->GetErrMsg() << std::endl;
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