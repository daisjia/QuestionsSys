#pragma once
#include<mysql/mysql.h>
#include<iostream>
#include<string>
#include<exception>
#include"mytime.h"

class Mysql
{
public:
	Mysql();
	~Mysql();

	MYSQL* mpcon;	//数据库
	MYSQL_RES* mp_res;	//返回值
	MYSQL_ROW mp_row;	//每一行

private:

};