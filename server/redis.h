#pragma once
#include<stdlib.h>
#include<stddef.h>
#include"log.h"
#include<string>
#include<hiredis/hiredis.h>
#include<iostream>
#include<string>
#include<mutex>
#include<map>
#include<atomic>
#include<queue>
#include<string.h>
#include<unistd.h>
#include<iostream>

#if 0
#define MAXSIZE 10

class RedisPool
{
public:
	static RedisPool* GetRedisPool()
	{
		if (redis == NULL)
		{
			objectLock.lock();
			if (redis == NULL)
			{
				redis = new RedisPool();
			}
			objectLock.unlock();
		}
		return redis;
	}

	void SetConf(int size = 4);
	~RedisPool();
	bool Query(const char* redisCmd, std::map<std::string, std::string>& result);
	bool Insert(const char* redisCmd);
	int Exist(const char* redisCmd);
	bool Del(const char* redisCmd);
private:
	RedisPool();
	bool CreateOneConnect();
	redisContext* GetOneConnect();
	void Close(redisContext* mpcon);

private:
	std::queue<redisContext *> _redisPool;
	std::atomic_int _size;
	static std::mutex objectLock;
	static std::mutex poolLock;
	static RedisPool* redis;
};

#endif


class Redis
{
public:
	~Redis();
	Redis(const char* ip, int port);
	void ReConnect();
	bool Query(const char* redisCmd, std::map<std::string, std::string>& result);
	bool Insert(const char* redisCmd);
	int Exist(const char* redisCmd);
	bool ExeCmd(const char* redisCmd);
	bool Del(const char* redisCmd);
private:
	redisContext* _redis;
	std::string _ip;
	int _port;
	std::mutex mtx;
};