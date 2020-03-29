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

/*
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
*/

static const uint16_t crc16tab[256] = {
	0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
	0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
	0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
	0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
	0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
	0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
	0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
	0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
	0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
	0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
	0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
	0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
	0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
	0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
	0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
	0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
	0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
	0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
	0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
	0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
	0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
	0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
	0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
	0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
	0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
	0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
	0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
	0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
	0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
	0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
	0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
	0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0
};

uint16_t crc16(const char* buf, int len)
{
	int counter;
	uint16_t crc = 0;
	for (counter = 0; counter < len; counter++)
		crc = (crc << 8) ^ crc16tab[((crc >> 8) ^ *buf++) & 0x00FF];
	return crc;
}

unsigned int Hash(std::string key)
{
	int keylen = key.size();
	int s, e; /* start-end indexes of { and } */

	for (s = 0; s < keylen; s++)
		if (key[s] == '{') break;

	/* No '{' ? Hash the whole key. This is the base case. */
	if (s == keylen) return crc16(key.data(), keylen) & 0x3FFF;

	/* '{' found? Check if we have the corresponding '}'. */
	for (e = s + 1; e < keylen; e++)
		if (key[e] == '}') break;

	/* No '}' or nothing betweeen {} ? Hash the whole key. */
	if (e == keylen || e == s + 1) return crc16(key.data(), keylen) & 0x3FFF;

	/* If we are here there is both a { and a } on its right. Hash
	 * what is in the middle between { and }. */
	return crc16(key.data() + s + 1, e - s - 1) & 0x3FFF;
}