#include "redis.h"

Redis* Redis::redis = NULL;
std::mutex Redis::mux;

Redis::Redis()
{
	_redis = redisConnect("127.0.0.1", 6379);
	if (_redis->err)
	{
		redisFree(_redis);
		LOG("connect to reidsServer fail!");
		return;
	}
	LOG("connect to redisServer success!");
}

bool Redis::RedisCommand(const std::string command, redisReply*(&reply))
{
	reply = (redisReply*)redisCommand(_redis, command.c_str());
	if (reply == NULL)
	{
		freeReplyObject(reply);
		char buff[100] = { 0 };
		sprintf(buff, "redis execut [ %s ] fail!", command.c_str());
		LOG(buff);
		return false;
	}
	if (reply->type == REDIS_REPLY_ERROR)
	{
		LOG(reply->str);
		freeReplyObject(reply);
		return false;
	}
	return true;
}
