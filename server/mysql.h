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

	MYSQL* mpcon;	//���ݿ�
	MYSQL_RES* mp_res;	//����ֵ
	MYSQL_ROW mp_row;	//ÿһ��

private:

};