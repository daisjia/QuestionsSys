#include "mysql.h"
Mysql * Mysql::mysql = NULL;
std::mutex Mysql::mux;

Mysql::Mysql()
{
	mpcon = mysql_init((MYSQL*)0);
	if (mpcon == NULL)
	{
		LOGE("mysql init fail!");
		return;
	}

	if (!mysql_real_connect(mpcon, "127.0.0.1", "root", "123456", NULL, 3306, NULL, 0))	//³É¹¦·µ»Ø0
	{
		LOGE("mysql connect error!");
		return;
	}

	if (mysql_select_db(mpcon, "item"))
	{
		LOGE("database select fail");
		return;
	}
	LOGI("connect to mysqlServer success!")
}

Mysql::~Mysql()
{
	if (NULL != mp_res)
	{
		mysql_free_result(mp_res);
	}
	mysql_close(mpcon);
}
