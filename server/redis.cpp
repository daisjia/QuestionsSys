#include "redis.h"

Redis* Redis::redis = NULL;
std::mutex Redis::mux;

Redis::Redis()
{
	_redis = redisConnect("127.0.0.1", 6379);
	if (_redis->err)
	{
		redisFree(_redis);
		LOGE("connect to reidsServer fail!");
		exit(0);
	}
	LOGI("connect to redisServer success!");
}

bool Redis::RedisCommand(const std::string command, redisReply*(&reply))
{
	reply = (redisReply*)redisCommand(_redis, command.c_str());
	if (reply == NULL)
	{
		freeReplyObject(reply);
		LOGE("redis execut [ %s ] fail!", command.c_str());
		return false;
	}
	if (reply->type == REDIS_REPLY_ERROR)
	{
		LOGE(reply->str);
		freeReplyObject(reply);
		return false;
	}
	return true;
}
