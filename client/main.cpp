#include<stdlib.h>
#include<stddef.h>
#include<string>
#include<hiredis/hiredis.h>
#include<iostream>
using namespace std;
#include"human.pb.h"
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<string.h>
#include<memory>
enum TYPE
{
	REGISTER,
	LOGIN,
};

int main()
{
	int port;
	cout << "please input port: ";
	cin >> port;
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	saddr.sin_port = htons(port);
	connect(fd, (sockaddr*)&saddr, sizeof(saddr));
	HumanValue::Person person;
	person.set_id(456);
	person.set_name("jiadai");
	person.set_passwd("123456");
	person.set_type(HumanValue::Person_HumanType::Person_HumanType_Student);
	std::string str;
	person.SerializeToString(&str);
	send(fd, str.c_str(), 1024, 0);
	char buff1[1024] = { 0 };
	int n = recv(fd, buff1, 1024, 0);
	string message = buff1;
	
	HumanValue::Result res;
	res.ParseFromString(message);
	int ret = res.ret();
	string rsp = res.rsp();
	cout << ret << "  " << rsp << endl;
}

#if 0
class Redis
{
public:
	static Redis* GetRedis()
	{
			if (redis == NULL)
			{
				redis = new Redis();
			}
		return redis;
	}
	bool RedisCommand(const std::string command);
public:
	redisContext* _redis;
	redisReply* _reply;
	static Redis* redis;
private:
	Redis();
};

Redis* Redis::redis = NULL;

Redis::Redis()
{
	_redis = redisConnect("127.0.0.1", 6379);
	if (_redis->err)
	{
		redisFree(_redis);
		cout<<"connect to reidsServer fail!"<<endl;
		return;
	}
	cout<<"connect to redisServer success!"<<endl;
}

bool Redis::RedisCommand(const std::string command)
{
	_reply = (redisReply*)redisCommand(_redis, command.c_str());
	if (_reply == NULL)
	{
		freeReplyObject(_reply);
		char buff[100] = { 0 };
		sprintf(buff, "redis execut [ %s ] fail!", command.c_str());
		cout << buff << endl;
		return false;
	}
	return true;
}

int main()
{
	Redis* redis = Redis::GetRedis();
	redis->RedisCommand("select 0");
	redis->RedisCommand("hmset user id 5 name jiadai");
	redis->RedisCommand("hgetall user");
	cout << "====" << redis->_reply->len << "   " << redis->_reply->elements << endl;
	for (int i = 0; i < redis->_reply->elements; ++i)
	{
		cout << redis->_reply->element[i]->str <<"   "<<redis->_reply->element[i]->len<< endl;
	}
	redis->RedisCommand("get jia");
	cout << redis->_reply->len << endl;

	return 0;
}
#endif