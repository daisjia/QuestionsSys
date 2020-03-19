#include "mysql.h"

std::mutex MysqlPool::objectLock;
std::mutex MysqlPool::poolLock;
MysqlPool* MysqlPool::mysql = NULL;

void MysqlPool::SetConf(const char* host, const char* user, const char* passwd, const char* database, int size)
{
	_host = host;
	_user = user;
	_passwd = passwd;
	_database = database;
	_size = 0;

	if (size > MAXSIZE || size <= 0)
	{
		LOGE("size error, create mysqlPool fail!");
		exit(0);
	}

	int loop = size;
	for (int i = 0; i < loop; ++i)
	{
		if (CreateOneConnect())
		{
			std::cout << "----------------Mysql id : "<<i<< " Create seccess!----------------" << std::endl;
		}
		else
		{
			std::cout << "----------------Mysql id : " << i << " Create fail!----------------" << std::endl;
		}
	}
}

MysqlPool::~MysqlPool()
{
	std::unique_lock<std::mutex> lck(poolLock);
	for (int i = 0; i < _size; ++i)
	{
		mysql_close(_mysqlPool.front());
		_mysqlPool.pop();
	}
	_size = 0;
}

bool MysqlPool::Query(const char* sql, std::map<std::string, std::vector<std::string>> &result)
{
	MYSQL* mpcon = GetOneConnect();
	if (mpcon == NULL)
	{
		sleep(1);
		mpcon = GetOneConnect();
		if (mpcon == nullptr)
		{
			LOGI("Get Mysql Connect fail, query fail!");
			return false;
		}
	}
	if (mysql_query(mpcon, sql) == 0)
	{
		MYSQL_RES* res = mysql_store_result(mpcon);
		if (res)
		{
			MYSQL_FIELD* field;
			while ((field = mysql_fetch_field(res)))
			{
				result.insert(std::make_pair(field->name, std::vector<std::string>()));
			}
			MYSQL_ROW row;
			while ((row = mysql_fetch_row(res)))
			{
				int i = 0;
				for (auto &it : result)
				{
					it.second.push_back(row[i++]);
				}
			}
			mysql_free_result(res);
			Close(mpcon);
			return true;
		}
		else
		{
			if (mysql_field_count(mpcon) != 0)
			{
				LOGI("mysql error: %s", mysql_errno(mpcon));
				Close(mpcon);
				return false;
			}
			Close(mpcon);
			return true;
		}
	}
	else
	{
		LOGI("sql error!");
		Close(mpcon);
		return false;
	}
}

bool MysqlPool::Insert(const char* sql)
{
	MYSQL* mpcon = GetOneConnect();
	if (mpcon == NULL)
	{
		LOGI("Get Mysql Connect fail, query fail!");
		Close(mpcon);
		return false;
	}

	if (mysql_query(mpcon, sql))
	{
		LOGI("mysql error: %s", mysql_errno(mpcon));
		Close(mpcon);
		return false;
	}
	Close(mpcon);
	return true;
}

MysqlPool::MysqlPool()
{
}

bool MysqlPool::CreateOneConnect()
{
	if (_size >= MAXSIZE)
	{
		LOGI("size too large, create fail!");
		return false;
	}
	std::unique_lock<std::mutex> lck(poolLock);
	MYSQL* mpcon = mysql_init(nullptr);
	if (mpcon == NULL)
	{
		LOGE("mysql init fail!");
		return false;
	}

	int time_out = 1;
	mysql_options(mpcon, MYSQL_OPT_RECONNECT, (const char*)&time_out);
	//Ê§°Ü·µ»Ø0

	if (!mysql_real_connect(mpcon, _host.c_str(), _user.c_str(), _passwd.c_str(), _database.c_str(), 3306, NULL, 0))
	{
		LOGE("mysql connect error!");
		return false;
	}
	_size++;
	_mysqlPool.push(mpcon);
	return true;
}

MYSQL* MysqlPool::GetOneConnect()
{
	std::unique_lock<std::mutex> lck(poolLock);
	if (_size <= 0)
	{
		LOGI("mysqlPool empty!");
		return nullptr;
	}
	MYSQL* mpcon = nullptr;
	int loop = _size;
	for (int i = 0; i < loop; ++i)
	{
		mpcon = _mysqlPool.front();
		_mysqlPool.pop();
		_size--;
		if (mysql_ping(mpcon))
		{
			if (!Reconnect(mpcon))
			{
				continue;
			}
			return mpcon;
		}
		return mpcon;
	}
	return nullptr;
}

void MysqlPool::Close(MYSQL* mpcon)
{
	if (mpcon != NULL)
	{
		std::unique_lock<std::mutex> lck(poolLock);
		_mysqlPool.push(mpcon);
		_size++;
	}
}

bool MysqlPool::Reconnect(MYSQL* mpcon)
{
	mysql_close(mpcon);
	int time_out = 1;
	mysql_options(mpcon, MYSQL_OPT_RECONNECT, (const char*)&time_out);
	//Ê§°Ü·µ»Ø0
	if (!mysql_real_connect(mpcon, _host.c_str(), _user.c_str(), _passwd.c_str(), _database.c_str(), 3306, NULL, 0))
	{
		LOGE("mysql Reconnect error!");
		return false;
	}
	return true;
}


