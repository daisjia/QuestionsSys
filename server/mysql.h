#pragma once
#include<mysql/mysql.h>
#include<iostream>
#include<string>
#include<exception>
#include<mutex>
#include<queue>
#include"log.h"
#include<atomic>
#include<map>
#include<unistd.h>


#define MAXSIZE 10

class MysqlPool
{
public:
	static MysqlPool* GetMysqlPool()
	{
		if (mysql == NULL)
		{
			objectLock.lock();
			if (mysql == NULL)
			{
				mysql = new MysqlPool();
			}
			objectLock.unlock();
		}
		return mysql;
	}

	void SetConf(const char* host, const char* user, const char* passwd, const char* database, int size);
	~MysqlPool();
	bool Query(const char* sql, std::map<std::string, std::vector<std::string>>& result);
	bool Insert(const char* sql);
private:
	MysqlPool();
	bool CreateOneConnect();
	MYSQL* GetOneConnect();
	void Close(MYSQL* mpcon);
	bool Reconnect(MYSQL *mpcon);


private:
	std::queue<MYSQL*> _mysqlPool;
	std::string _host;
	std::string _user;
	std::string _passwd;
	std::string _database;
	std::atomic_int _size;
	static std::mutex objectLock;
	static std::mutex poolLock;
	static MysqlPool* mysql;
};