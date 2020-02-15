#pragma once
#include<stdlib.h>
#include<stddef.h>
#include"mytime.h"
#include<string>
#include<hiredis/hiredis.h>
#include<mutex>

class Redis
{
public:
	static Redis* GetRedis()
	{
		if (redis == NULL)
		{
			mux.lock();
			if (redis == NULL)
			{
				redis = new Redis();
			}
			mux.unlock();
		}
		return redis;
	}
	bool RedisCommand(const std::string command, redisReply* (&reply));
public:
	redisContext* _redis;
	redisReply* _reply;
	static Redis* redis;
private: 
	Redis();
	static std::mutex mux;
};