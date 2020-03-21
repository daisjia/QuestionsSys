#include "balance.h"

std::mutex mtx;
static std::map<unsigned int, HashRing*> data;
static std::map<int, std::vector<unsigned int>> fd_hashid; //key是fd，value是hash_id， 用于删除虚节点时通过fd查找到所有的虚节点。
static std::set<int> serport; //存储所有服务器端口号，服务器端口是负载均衡分配
timeval time_out;
Balance::Balance()
{
	srand(time(NULL));
	gettimeofday(&time_out, NULL);
	time_out.tv_sec += 10;
	_serfd = socket(AF_INET, SOCK_STREAM, 0);
	_clifd = socket(AF_INET, SOCK_STREAM, 0);
	if (_serfd == -1 || _clifd == -1)
	{
		LOGI("balance socket fail!");
		exit(0);
	}

	struct sockaddr_in saddr, caddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(5000);
	saddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	int res = bind(_serfd, (struct sockaddr*) & saddr, sizeof(saddr));
	if (res == -1)
	{
		LOGI("balance server bind fail!");
		exit(0);
	}

	caddr.sin_family = AF_INET;
	caddr.sin_port = htons(5001);
	caddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	res = bind(_clifd, (struct sockaddr*) & caddr, sizeof(caddr));
	if (res == -1)
	{
		LOGI("balance client fail!");
		exit(0);
	}

	res = listen(_serfd, 20);
	if (res == -1)
	{
		LOGI("balance listen fail!");
		exit(0);
	}

	res = listen(_clifd, 20);
	if (res == -1)
	{
		LOGI("balance listen fail!");
		exit(0);
	}

	_base = event_base_new();
	for (int i = 0; i < 4; ++i)
	{
		mythread.push_back(std::thread(DealClient, _clifd));
	}

	mythread.push_back(std::thread(HeartRate, nullptr));
	LOGI("balance start run");
}

Balance::~Balance()
{
	for (auto& t : mythread)
		t.join();

	for (auto it : data)
	{
		if (it.second->flag == REAL)
			delete it.second->node;

		it.second->node = nullptr;
		delete it.second;
	}
	data.clear();
	fd_hashid.clear();
	serport.clear();
}

void Balance::Run()
{
	struct event* env = event_new(_base, _serfd, EV_READ | EV_WRITE | EV_PERSIST, DealServer, this);
	event_add(env, NULL);
	event_base_dispatch(_base);
}

void DealServer(int fd, short event, void* arg)
{
	Balance* b = static_cast<Balance*>(arg);
	sockaddr_in saddr;
	socklen_t len = sizeof(saddr);
	int serfd = accept(fd, (struct sockaddr*) & saddr, &len);
	if (serfd <= 0)
	{
		LOGI("server accept fail!");
		return;
	}

	std::string ip = inet_ntoa(saddr.sin_addr);
	int port = rand() % 10000 + 5002;	//服务器端口由负载均衡分配
	auto portIt = serport.find(port);
	while (portIt != serport.end())
	{
		port = rand() % 10000 + 5002;
		portIt = serport.find(port);
	}

	/*
		如果实环的结点冲突则直接close，并退出，不添加这台服务器，
		如果虚环结点冲突则忽略，添加的虚结点个数不一定是4个
	*/
	std::string hash = ip + std::to_string(port);
	unsigned int hash_id = Hash(hash);
	auto it = data.find(hash_id);
	if (it != data.end())
	{
		close(serfd);
		return;
	}

	int res = send(serfd, (void*)&port, 4, 0);
	if (res <= 0)
	{
		close(serfd);
		return;
	}
	serport.insert(port);


	HashNode* node = new HashNode(serfd, ip, port);
	HashRing* realring = new HashRing();
	realring->flag = REAL;
	realring->node = node;
	data.insert(std::make_pair(hash_id, realring));
	fd_hashid[serfd].push_back(hash_id);

	LOGI("server connect ip: %s, port: %d", ip.c_str(), port);

	for (int i = 0; i < 4; ++i)
	{
		hash.clear();
		hash = ip + std::to_string(port) + std::to_string(i);
		hash_id = Hash(hash);
		it = data.find(hash_id);
		if (it != data.end())
		{
			continue;
		}
		HashRing* virtualring = new HashRing();
		virtualring->flag = VRITUAL;
		virtualring->node = node;
		data.insert(std::make_pair(hash_id, virtualring));
		fd_hashid[serfd].push_back(hash_id);
	}
}

void DealClient(int fd)
{
	while (true)
	{
		sockaddr_in caddr;
		socklen_t len = sizeof(caddr);
		int clifd = 0;
		do
		{
			std::unique_lock<std::mutex> lck(mtx);
			if (data.size() == 0)
			{
				break;
			}
			clifd = accept(fd, (struct sockaddr*) & caddr, &len);
			LOGI("client ip: %s, port: %d up", inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port));
			if (clifd <= 0)
			{
				break;
			}
		} while (0);

		if (data.size() == 0 || clifd <= 0)
		{
			continue;
		}

		std::string ip = inet_ntoa(caddr.sin_addr);
		int port = ntohs(caddr.sin_port);
		std::string hash = ip + std::to_string(port);
		unsigned int hash_id = Hash(hash);

		auto it = data.lower_bound(hash_id);
		if (it == data.end())
		{
			it = data.begin();
		}

		Json::Value RspBuffer;
		RspBuffer["ip"] = it->second->node->ip.data();
		RspBuffer["port"] = it->second->node->port;
		send(clifd, RspBuffer.toStyledString().c_str(), RspBuffer.toStyledString().size(), 0);
		close(clifd);
	}

}

void timeout_cb()
{
	for (auto it = fd_hashid.begin(); it != fd_hashid.end(); ++it)
	{
		int res = send(it->first, "PING", 5, 0);
		if (res <= 0)
		{
			close(it->first);
			for (auto it1 : it->second)
			{
				if (data[it1]->flag == REAL)
				{
					LOGI("server ip: %s, port: %d done!", data[it1]->node->ip.data(), data[it1]->node->port);
					serport.erase(data[it1]->node->port);
					delete data[it1]->node;
				}
				data[it1]->node = nullptr;
				delete data[it1];
				data.erase(it1);
			}
			it->second.clear();
			fd_hashid.erase(it++);
			if (fd_hashid.size() == 0)
			{
				break;
			}
		}

		char Buffer[5] = { 0 };
		res = recv(it->first, Buffer, 5, 0);
		if (res <= 0)
		{
			close(it->first);
			for (auto it1 : it->second)
			{
				if (data[it1]->flag == REAL)
				{
					LOGI("server ip: %s, port: %d done!", data[it1]->node->ip.data(), data[it1]->node->port);
					serport.erase(data[it1]->node->port);
					delete data[it1]->node;
				}
				data[it1]->node = nullptr;
				delete data[it1];
				data.erase(it1);
			}
			it->second.clear();
			fd_hashid.erase(it++);
			if (fd_hashid.size() == 0)
			{
				break;
			}
		}
		LOGI("ip: %s, port: %d %s", data[it->second.front()]->node->ip.data(), data[it->second.front()]->node->port, Buffer);
	}
}

void HeartRate(void* arg)
{
	while (true)
	{
		timeval tv;
		gettimeofday(&tv, NULL);
		if (tv.tv_sec > time_out.tv_sec || (tv.tv_sec == time_out.tv_sec && tv.tv_usec > time_out.tv_usec))
		{
			timeout_cb();
			gettimeofday(&time_out, NULL);
			time_out.tv_sec += 10;
		}
	}
}

unsigned int Hash(std::string str)
{
	static unsigned int hash = 0;
	for (int i = 0; i < str.size(); ++i)
	{
		hash = str[i] + (hash << 6) + (hash << 16) - hash;
	}
	hash++;
	return (hash & 0x7FFFFFFF);
}
