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
	Redis(std::string, int port);
	bool RedisCommand(const std::string command);
public:
	redisContext* _redis;
	redisReply* _reply;
	std::mutex _mtx;
};