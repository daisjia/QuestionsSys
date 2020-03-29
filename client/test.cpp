#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <poll.h>
#include <unistd.h>
#include<event.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string>
#include <vector>
#include <map>
#include"pdu.h"
#include"IM.User.Msg.pb.h"
#include<vector>
#include<fcntl.h>
#include<pthread.h>
#include<json/json.h>
#include<iostream>
using namespace std;
bool Read(int sockfd);

int setnonblocking(int fd)
{
	int old_opt = fcntl(fd, F_GETFL);
	int new_opt = old_opt | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_opt);
}

void addfd(int epoll_fd, int fd)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLOUT | EPOLLET | EPOLLERR;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
	setnonblocking(fd);
}

static int id = 0;
bool Write(int sockfd)
{
	//cout << "***************: " << id << endl;
	//int id = rand() % 99999 + 1;
	char name[20];
	string passwd = "123456";
	int type = 1;
	sprintf(name, "jiadai%d", id++);
	PDUHEAD pReqHeader;
	pReqHeader.SetCmd(REGISTER);
	char ReqBuffer[1024 * 1024] = { 0 };
	IM::User::Msg::IMRegisterReq Msg;
	Msg.set_id(id);
	Msg.set_name(name);
	Msg.set_passwd(passwd.c_str());
	Msg.set_type(type);
	std::string ReqMsg;
	Msg.SerializeToString(&ReqMsg);
	memcpy(ReqBuffer, (void*)&pReqHeader, pReqHeader.GetHeadLen());
	memcpy(ReqBuffer + pReqHeader.GetHeadLen(), ReqMsg.c_str(), ReqMsg.size());
	((PDUHEAD*)ReqBuffer)->SetPackLen(pReqHeader.GetHeadLen() + ReqMsg.size());
	int ret = send(sockfd, ReqBuffer, pReqHeader.GetHeadLen() + ReqMsg.size(), 0);
	if (ret <= 0)
	{
		return false;
	}
//	ret = Read(sockfd);
	if (ret <= 0)
	{
		return false;
	}
	return true;
}

bool Read(int sockfd)
{
	char RspBuffer[1024 * 1024] = { 0 };
	int res = recv(sockfd, RspBuffer, 1024 * 1024, 0);
	if (res <= 0)
	{
		return false;
	}
	IM::User::Msg::IMRspMsg Msg2;
	PDUHEAD* pRspHeader = (PDUHEAD*)RspBuffer;
	std::string RspMsg = pRspHeader->GetBody();
	Msg2.ParseFromString(RspMsg);
	int ret = Msg2.ret();
	std::string msg = Msg2.msg();
	cout << "sockfd: " << sockfd << " buffer: " << msg << endl;
	return true;
}

void start_conn(int epoll_fd, int num, const char* ip, int port)
{
	int ret = 0;
	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &address.sin_addr);
	address.sin_port = htons(port);
	for (int i = 0; i < num; ++i)
	{
		int sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
		{
			continue;
		}
		if (connect(sockfd, (struct sockaddr*) & address, sizeof(address)) == 0)
		{
			char RspBuffer[1024] = { 0 };
			Json::Value val;
			Json::Reader reader;
			int res = recv(sockfd, RspBuffer, 1024, 0);
			if (res <= 0)
			{
				close(sockfd);
				continue;
			}

			if (reader.parse(RspBuffer, val) == -1)
			{
				close(sockfd);
				continue;
			}

			std::string ip1 = val["ip"].asString();
			int port1 = val["port"].asInt();
			if (port1 < 5002)
			{
				close(sockfd);
				continue;
			}
			close(sockfd);
			sockfd = socket(AF_INET, SOCK_STREAM, 0);
			struct sockaddr_in address1;
			memset(&address1, 0, sizeof(address1));
			address1.sin_family = AF_INET;
			inet_pton(AF_INET, ip1.data(), &address1.sin_addr);
			address1.sin_port = htons(port1);
			connect(sockfd, (struct sockaddr*) & address1, sizeof(address1));
			addfd(epoll_fd, sockfd);
		}
		cout << i << endl;
	}
}

void close_conn(int epollfd, int sockfd)
{
	cout << "close: " << sockfd << endl;
	epoll_ctl(epollfd, EPOLL_CTL_DEL, sockfd, 0);
	close(sockfd);
}


/*
void* pRun(void*arg)
{
	epoll_event events[100000];
	int epollfd = *((int *)arg);
	while (true)
	{
	//	cout << "===============A : size: " << a.vc.size() << std::endl;
		int fds = epoll_wait(epollfd, events, 100000, 200000);
		for (int i = 0; i < fds; i++)
		{
			int sockfd = events[i].data.fd;
			if (events[i].events & EPOLLIN)
			{
				if (!Write(sockfd))
				{
					close_conn(epollfd, sockfd);
				}
				struct epoll_event event;
				event.events = EPOLLOUT | EPOLLET | EPOLLERR;
				event.data.fd = sockfd;
				epoll_ctl(epollfd, EPOLL_CTL_MOD, sockfd, &event);
			}
			else if (events[i].events & EPOLLOUT)
			{
				if (!Read(sockfd))
				{
					close_conn(epollfd, sockfd);
				}
				struct epoll_event event;
				event.events = EPOLLOUT | EPOLLET | EPOLLERR;
				event.data.fd = sockfd;
				epoll_ctl(epollfd, EPOLL_CTL_MOD, sockfd, &event);
			}
			else if (events[i].events & EPOLLERR)
			{
				close_conn(epollfd, sockfd);
			}
		}
	}
}
*/
int main()
{
	srand(time(NULL));
	int num = 0;
	int port = 0;
	cout << "please input num: ";
	cin >> num;
	int epollfd = epoll_create(100);
	start_conn(epollfd, num, "127.0.0.1", 5001);
	epoll_event events[100000];
	pthread_t id;
	//pthread_create(&id, NULL, pRun, (void*)&epollfd);
	int a = 0;
	while (true)
	{
		/*if (a == 10)
			break;
		a++;*/
		//cout << "===============A : size: " << a.vc.size() << std::endl;
		int time = rand() % 3 + 0;
		sleep(time);
		int fds = epoll_wait(epollfd, events, 1000, 200000);
		for (int i = 0; i < fds; i++)
		{
			int sockfd = events[i].data.fd;
			if (events[i].events & EPOLLIN)
			{
				if (!Read(sockfd))
				{
					close_conn(epollfd, sockfd);
				}
				struct epoll_event event;
				event.events = EPOLLOUT | EPOLLET | EPOLLERR;
				event.data.fd = sockfd;
				epoll_ctl(epollfd, EPOLL_CTL_MOD, sockfd, &event);
			}
			else if (events[i].events & EPOLLOUT)
			{
				if (!Write(sockfd))
				{
					close_conn(epollfd, sockfd);
				}
				Read(sockfd);
				struct epoll_event event;
				event.events = EPOLLOUT | EPOLLET | EPOLLERR;
				event.data.fd = sockfd;
				epoll_ctl(epollfd, EPOLL_CTL_MOD, sockfd, &event);
			}
			else if (events[i].events & EPOLLERR)
			{
				close_conn(epollfd, sockfd);
			}
		}
	}
	return 0;
}