#include "control.h"

Redis* redis = Redis::GetRedis();
Mysql* mysql = Mysql::GetMysql();

void SendResult(int fd, int ret, std::string message)
{
	if (ret == 0)
	{
		LOG("ret can not equal 0");
		return;
	}
	HumanValue::Result rsp;
	rsp.set_ret(ret);
	rsp.set_rsp(message.c_str());
	std::string str;
	rsp.SerializeToString(&str);
	send(fd, str.c_str(), 1024, 0);
}

Control::Control()
{
	_model.insert(std::make_pair(REGISTER, Register));
	_model.insert(std::make_pair(LOGIN, Login));
}

Control::~Control()
{
}

bool Register(int fd, std::string message)
{
	//std::cout << message << std::endl;
	HumanValue::Person person;
	person.ParseFromString(message);
	int id = person.id();
	std::string name = person.name();
	std::string passwd = person.passwd();
	int type = person.type();

	char buff[100] = { 0 };
	sprintf(buff, "hget user:id:%d id", id);
	redisReply* reply;
	if (!redis->RedisCommand(buff, reply))
	{
		SendResult(fd, -1, "redis command fail!");
		freeReplyObject(reply);
		return false;
	}
	if (reply->type != REDIS_REPLY_NIL)
	{
		freeReplyObject(reply);
		SendResult(fd, -1, "user has exist!");
		return false;
	}
	freeReplyObject(reply);
	char cmd[100] = { 0 };
	sprintf(cmd, "select id from user where id = %d", id);
	if (mysql_real_query(mysql->mpcon, cmd, strlen(cmd)))
	{
		LOG("mysql select fail!");
		SendResult(fd, -1, "mysql select fail!");
		return false;
	}
	MYSQL_RES *mp_res;
	MYSQL_ROW mp_row;
	mp_res = mysql_store_result(mysql->mpcon);
	mp_row = mysql_fetch_row(mp_res);

	if (mp_row != 0)
	{
		SendResult(fd, -1, "user has exist!");
		return false;
	}
	
	sprintf(cmd, "insert into user value (%d, \"%s\", \"%s\", %d)", id, name.c_str(), passwd.c_str(), type);
	if (mysql_real_query(mysql->mpcon, cmd, strlen(cmd)))
	{
		LOG("mysql insert fail!");
		SendResult(fd, -1, "mysql insert fail!");
		return false;
	}
	
	SendResult(fd, 1, "user register success!");
	return true;
}

bool Login(int fd, std::string message)
{
	return false;
}


