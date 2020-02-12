#include "redis.h"

Redis::Redis(std::string ip, int port)
{
	_redis = redisConnect(ip.c_str(), port);
	if (_redis->err)
	{
		redisFree(_redis);
		LOG("connect to reidsServer fail!");
		return;
	}
	LOG("connect to redisServer success!");
}

bool Redis::RedisCommand(const std::string command)
{
	std::unique_lock<std::mutex> lok(_mtx);
	_reply = (redisReply*)redisCommand(_redis, command.c_str());
	if (_reply == NULL)
	{
		freeReplyObject(_reply);
		char buff[100] = { 0 };
		sprintf(buff, "redis execut [ %s ] fail!", command.c_str());
		LOG(buff);
		return false;
	}
	return true;
}
