#pragma once
#include<mysql/mysql.h>
#include<iostream>
#include<string>
#include<exception>
#include<mutex>
#include"mytime.h"

class Mysql
{
public:
	static Mysql* GetMysql()
	{
		if (mysql == NULL)
		{
			mux.lock();
			if (mysql == NULL)
			{
				mysql = new Mysql();
			}
			mux.unlock();
		}
		return mysql;
	}
	~Mysql();

	MYSQL* mpcon;	//数据库
	MYSQL_RES* mp_res;	//返回值
	MYSQL_ROW mp_row;	//每一行
	static Mysql* mysql;
private:
	
	static std::mutex mux;
	Mysql();
};