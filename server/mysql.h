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

	MYSQL* mpcon;	//���ݿ�
	MYSQL_RES* mp_res;	//����ֵ
	MYSQL_ROW mp_row;	//ÿһ��
	static Mysql* mysql;
private:
	
	static std::mutex mux;
	Mysql();
};