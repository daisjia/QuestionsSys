#include "redis.h"


std::mutex RedisPool::objectLock;
std::mutex RedisPool::poolLock;
RedisPool* RedisPool::redis = NULL; 

void RedisPool::SetConf(int size)
{
	_size = 0;
	if (size > MAXSIZE || size <= 0)
	{
		LOGE("size error, create redisPool fail!");
		exit(0);
	}

	int loop = size;
	for (int i = 0; i < loop; ++i)
	{
		if (CreateOneConnect())
		{
			std::cout << "----------------Redis id : " << i << "Create seccess!----------------" << std::endl;
		}
		else
		{
			std::cout << "----------------Redis id : " << i << "Create fail!----------------" << std::endl;
		}
	}
}

RedisPool::~RedisPool()
{
	std::unique_lock<std::mutex> lck(poolLock);
	int loop = _size;
	for (int i = 0; i < loop; ++i)
	{
		if (_redisPool.front() == NULL)
		{
			_redisPool.pop();
			continue;
		}

		redisFree(_redisPool.front());
		_redisPool.pop();
	}
	_size = 0;
}

//用来执行hash的，hgetall
bool RedisPool::Query(const char* redisCmd, std::map<std::string, std::string>& result)
{
	redisContext* r = GetOneConnect();
	if (r == NULL)
	{
		sleep(1);
		r = GetOneConnect();
		if (r == nullptr)
		{
			LOGI("get redis connect fail!");
			return false;
		}
	}

	redisReply* reply = (redisReply*)redisCommand(r, redisCmd);
	if (reply == NULL)
	{
		Close(r);
		LOGI("redisCommand error!");
		return false;
	}
	if (reply->type == REDIS_REPLY_ERROR)
	{
		LOGE("redis exn error!");
		freeReplyObject(reply);
		Close(r);
		return false;
	}

	if (reply->type == REDIS_REPLY_ARRAY)
	{
		for (int i = 0; i < reply->elements;)
		{
			result.insert(std::make_pair(reply->element[i]->str, reply->element[i + 1]->str));
			i += 2;
		}
		freeReplyObject(reply);
		Close(r);
		return true;
	}

	if (reply->type == REDIS_REPLY_NIL)
	{
		result.clear();
		freeReplyObject(reply);
		Close(r);
		return true;
	}

	Close(r);
	return false;
}

bool RedisPool::Insert(const char* redisCmd)
{
	redisContext* r = GetOneConnect();
	if (r == NULL)
	{
		sleep(1);
		r = GetOneConnect();
		if (r == nullptr)
		{
			LOGI("get redis connect fail!");
			return false;
		}
	}

	redisReply* reply = (redisReply*)redisCommand(r, redisCmd);
	if (reply == NULL)
	{
		Close(r);
		LOGI("redisCommand error!");
		return false;
	}
	if (reply->type == REDIS_REPLY_ERROR)
	{
		LOGE("redis command fail!");
		freeReplyObject(reply);
		Close(r);
		return false;
	}

	if (reply->type == REDIS_REPLY_STATUS)
	{
		if (!strcasecmp(reply->str, "ok"))
		{
			freeReplyObject(reply);
			Close(r);
			return true;
		}
		else
		{
			freeReplyObject(reply);
			Close(r);
			return false;
		}
	}
	freeReplyObject(reply);
	Close(r);
	return false;
}

RedisPool::RedisPool()
{
}

bool RedisPool::CreateOneConnect()
{
	std::unique_lock<std::mutex> lck(poolLock);
	if (_size >= MAXSIZE)
	{
		LOGE("size too large, create redisPool fail!");
		return false;
	}

	redisContext *r = redisConnect("127.0.0.1", 6379);
	if (r->err)
	{
		redisFree(r);
		LOGE("connect to reidsServer fail!");
		return false;
	}
	_redisPool.push(r);
	_size++;
	return true;
}

redisContext* RedisPool::GetOneConnect()
{
	std::unique_lock<std::mutex> lck(poolLock);
	if (_size <= 0)
	{
		LOGI("no redis can get!");
		return nullptr;
	}

	redisContext* r = _redisPool.front();
	_redisPool.pop();
	_size--;
	redisReply* reply = (redisReply*)redisCommand(r, "ping");
	if (reply == NULL)
	{
		LOGI("redisCommand ping error!");
		return false;
	}

	if (strcasecmp(reply->str, "PONG"))
	{
		return nullptr;
	}
	return r;
}

void RedisPool::Close(redisContext* r)
{
	std::unique_lock<std::mutex> lck(poolLock);
	_redisPool.push(r);
	_size++;
}
