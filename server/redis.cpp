#include "redis.h"


#if 0
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
			std::cout << "----------------Redis id : " << i << " Create seccess!----------------" << std::endl;
		}
		else
		{
			std::cout << "----------------Redis id : " << i << " Create fail!----------------" << std::endl;
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

	if (reply->type == REDIS_REPLY_INTEGER)
	{
		if (reply->integer == 0)
		{
			freeReplyObject(reply);
			Close(r);
			return false;
		}
		else
		{
			freeReplyObject(reply);
			Close(r);
			return true;
		}
	}
	freeReplyObject(reply);
	Close(r);
	return false;
}

int RedisPool::Exist(const char* redisCmd)
{
	redisContext* r = GetOneConnect();
	if (r == NULL)
	{
		sleep(1);
		r = GetOneConnect();
		if (r == nullptr)
		{
			LOGI("get redis connect fail!");
			return -1;
		}
	}

	redisReply* reply = (redisReply*)redisCommand(r, redisCmd);
	if (reply == NULL)
	{
		Close(r);
		LOGI("redisCommand error!");
		return -1;
	}
	if (reply->type == REDIS_REPLY_ERROR)
	{
		LOGE("redis command fail!");
		freeReplyObject(reply);
		Close(r);
		return -1;
	}

	if (reply->type == REDIS_REPLY_INTEGER)
	{
		if (reply->integer == 0)
		{
			freeReplyObject(reply);
			Close(r);
			return 0;
		}
		else
		{
			freeReplyObject(reply);
			Close(r);
			return 1;
		}
	}
	freeReplyObject(reply);
	Close(r);
	return -1;
}

bool RedisPool::Del(const char* redisCmd)
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
		LOGI("redis command fail!");
		freeReplyObject(reply);
		Close(r);
		return false;
	}

	if (reply->type == REDIS_REPLY_INTEGER)
	{
		if (reply->integer == 0)
		{
			freeReplyObject(reply);
			Close(r);
			return false;
		}
		else
		{
			freeReplyObject(reply);
			Close(r);
			return true;
		}
	}
	freeReplyObject(reply);
	Close(r);
	return false;
}

RedisPool::RedisPool()
{}

bool RedisPool::CreateOneConnect()
{
	std::unique_lock<std::mutex> lck(poolLock);
	if (_size >= MAXSIZE)
	{
		LOGE("size too large, create redisPool fail!");
		return false;
	}

	redisContext* r = redisConnect("127.0.0.1", 6379);
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

#endif

//*************************************

Redis::~Redis()
{
	redisFree(_redis);
	_redis = nullptr;
}

Redis::Redis(const char* ip, int port)
{
	_redis = redisConnect(ip, port);
	if (_redis->err)
	{
		redisFree(_redis);
		LOGI("connect to reidsServer fail!");
		exit(0);
	}
}

void Redis::ReConnect()
{
	if (_redis != nullptr)
	{
		redisFree(_redis);
	}
	_redis = redisConnect(_ip.c_str(), _port);
	if (_redis->err)
	{
		redisFree(_redis);
		LOGI("ReConnect to reidsServer fail!");
		exit(0);
	}
}

//用来执行hash的，hgetall
bool Redis::Query(const char* redisCmd, std::map<std::string, std::string>& result)
{
	std::unique_lock<std::mutex> lok(mtx);
	redisReply* reply = (redisReply*)redisCommand(_redis, "PING");
	if (reply != nullptr && reply->type == REDIS_REPLY_STATUS)
	{
		if (strcasecmp(reply->str, "PONG"))
		{
			ReConnect();
		}
	}
	else
	{
		redisFree(_redis);
		LOGI("Query connect to reidsServer fail!");
		return false;
	}
	freeReplyObject(reply);
	reply = (redisReply*)redisCommand(_redis, redisCmd);
	if (reply == NULL)
	{
		LOGI("redisCommand error!");
		return false;
	}
	if (reply->type == REDIS_REPLY_ERROR)
	{
		LOGI("redis exe error!");
		freeReplyObject(reply);
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
		return true;
	}

	if (reply->type == REDIS_REPLY_NIL)
	{
		result.clear();
		freeReplyObject(reply);
		return true;
	}
	return false;
}

bool Redis::Insert(const char* redisCmd)
{
	std::unique_lock<std::mutex> lok(mtx);
	redisReply* reply = (redisReply*)redisCommand(_redis, "PING");
	if (reply != nullptr && reply->type == REDIS_REPLY_STATUS)
	{
		if (strcasecmp(reply->str, "PONG"))
		{
			ReConnect();
		}
	}
	else
	{
		redisFree(_redis);
		LOGI("Insert connect to reidsServer fail!");
		exit(0);
	}
	freeReplyObject(reply);

	reply = (redisReply*)redisCommand(_redis, redisCmd);
	if (reply == NULL)
	{
		LOGI("redisCommand error!");
		return false;
	}
	if (reply->type == REDIS_REPLY_ERROR)
	{
		LOGI("redis command fail!");
		freeReplyObject(reply);
		return false;
	}

	if (reply->type == REDIS_REPLY_STATUS)
	{
		if (!strcasecmp(reply->str, "ok"))
		{
			freeReplyObject(reply);
			return true;
		}
		else
		{
			freeReplyObject(reply);
			return false;
		}
	}

	if (reply->type == REDIS_REPLY_INTEGER)
	{
		if (reply->integer == 0)
		{
			freeReplyObject(reply);
			return false;
		}
		else
		{
			freeReplyObject(reply);
			return true;
		}
	}
	freeReplyObject(reply);
	return false;
}

int Redis::Exist(const char* redisCmd)
{
	std::unique_lock<std::mutex> lok(mtx);
	redisReply* reply = (redisReply*)redisCommand(_redis, "PING");
	if (reply != nullptr && reply->type == REDIS_REPLY_STATUS)
	{
		if (strcasecmp(reply->str, "PONG"))
		{
			ReConnect();
		}
	}
	else
	{
		redisFree(_redis);
		LOGI("Exist connect to reidsServer fail!");
		exit(0);
	}
	freeReplyObject(reply);

	reply = (redisReply*)redisCommand(_redis, redisCmd);
	if (reply == NULL)
	{
		LOGI("redisCommand error!");
		return -1;
	}
	if (reply->type == REDIS_REPLY_ERROR)
	{
		LOGI("redis command fail!");
		freeReplyObject(reply);
		return -1;
	}

	if (reply->type == REDIS_REPLY_INTEGER)
	{
		if (reply->integer == 0)
		{
			freeReplyObject(reply);
			return 0;
		}
		else
		{
			freeReplyObject(reply);
			return 1;
		}
	}
	freeReplyObject(reply);
	return -1;
}

bool Redis::ExeCmd(const char* redisCmd)
{
	std::unique_lock<std::mutex> lok(mtx);
	redisReply* reply = (redisReply*)redisCommand(_redis, "PING");
	if (reply != nullptr && reply->type == REDIS_REPLY_STATUS)
	{
		if (strcasecmp(reply->str, "PONG"))
		{
			ReConnect();
		}
	}
	else
	{
		redisFree(_redis);
		LOGI("Insert connect to reidsServer fail!");
		exit(0);
	}
	freeReplyObject(reply);

	reply = (redisReply*)redisCommand(_redis, redisCmd);
	if (reply == NULL)
	{
		LOGI("redisCommand error!");
		return false;
	}
	if (reply->type == REDIS_REPLY_ERROR)
	{
		LOGI("redis command fail!");
		freeReplyObject(reply);
		return false;
	}

	return true;
}

bool Redis::Del(const char* redisCmd)
{
	std::unique_lock<std::mutex> lok(mtx);
	redisReply* reply = (redisReply*)redisCommand(_redis, "PING");
	if (reply != nullptr && reply->type == REDIS_REPLY_STATUS)
	{
		if (strcasecmp(reply->str, "PONG"))
		{
			ReConnect();
		}
	}
	else
	{
		redisFree(_redis);
		LOGI("Delete connect to reidsServer fail!");
		exit(0);
	}
	freeReplyObject(reply);
	reply = (redisReply*)redisCommand(_redis, redisCmd);
	if (reply == NULL)
	{
		LOGI("redisCommand error!");
		return false;
	}
	if (reply->type == REDIS_REPLY_ERROR)
	{
		LOGI("redis command fail!");
		freeReplyObject(reply);
		return false;
	}

	if (reply->type == REDIS_REPLY_INTEGER)
	{
		if (reply->integer == 0)
		{
			freeReplyObject(reply);
			return false;
		}
		else
		{
			freeReplyObject(reply);
			return true;
		}
	}
	freeReplyObject(reply);
	return false;
}
