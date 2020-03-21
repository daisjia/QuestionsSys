#pragma once
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
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string>
#include <vector>
#include<mutex>
#include <map>
#include<thread>
#include<event.h>
#include<set>
#include<mutex>
#include<time.h>
#include"log.h"
#include<json/json.h>

#define VRITUAL 0
#define REAL 1

struct HashNode
{
	int fd;
	std::string ip;
	int port;
	HashNode(int Fd = 0, std::string Ip = { "" }, int Port = -1) : fd(Fd), ip(Ip), port(Port) {}
};
struct HashRing
{
	int flag;	//0代表虚结点，1代表实结点
	HashNode* node;
	HashRing() : flag(0), node(nullptr) {}
	~HashRing() 
	{
		//用户自行删除node，防止内存泄露
		/*if (node != nullptr)
			delete node;
		node = nullptr;*/
	}
};

static void DealServer(int fd, short event, void* arg);
static void DealClient(int fd);
static void HeartRate(void* arg);
void timeout_cb();
unsigned int Hash(std::string str);

class Balance
{
public:
	Balance();
	~Balance();
	void Run();

	int GetSerFd()
	{
		return _serfd;
	}

	int GetCliFd()
	{
		return _clifd;
	}
private:
	int _serfd;
	int _clifd;
	std::vector<std::thread> mythread;
	struct event_base* _base;
};