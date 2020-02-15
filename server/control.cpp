#include "control.h"

Redis* redis = Redis::GetRedis();
Mysql* mysql = Mysql::GetMysql();

void SendResult(int fd, int cmd, int ret,std::string ReqMsg)
{
	IM::User::Msg::IMRspMsg Msg;
	Msg.set_ret(ret);
	Msg.set_msg(ReqMsg.c_str());
	std::string RspMsg;
	Msg.SerializeToString(&RspMsg);
	char RspBuffer[1024 * 1024] = { 0 };
	PDUHEAD pReqHeader;
	pReqHeader.SetCmd(cmd);
	memcpy(RspBuffer, (void*)&pReqHeader, pReqHeader.GetHeadLen());
	memcpy(RspBuffer + pReqHeader.GetHeadLen(), RspMsg.c_str(), RspMsg.size());
	((PDUHEAD*)RspBuffer)->SetPackLen(pReqHeader.GetHeadLen() + RspMsg.size());
	send(fd, RspBuffer, pReqHeader.GetHeadLen() + RspMsg.size(), 0);
}

Control::Control()
{
	_model.insert(std::make_pair(REGISTER, Register));
	_model.insert(std::make_pair(LOGIN, Login));
}

Control::~Control()
{
}

bool Register(int fd, std::string ReqMsg)
{
	PDUHEAD* pReqHeader = (PDUHEAD*)ReqMsg.c_str();
	std::string req_body = pReqHeader->GetBody();
	IM::User::Msg::IMRegisterReq user;
	user.ParseFromString(req_body);
	int id = user.id();
	std::string name = user.name();
	std::string passwd = user.passwd();
	int type = user.type();

	char redisCmd[100] = { 0 };
	sprintf(redisCmd, "hget user:id:%d id", id);
	redisReply* reply;
	if (!redis->RedisCommand(redisCmd, reply))
	{
		LOGE("redis command fail!");
		SendResult(fd, REGISTER, IM_ERROR, "server error, register fail!");
		freeReplyObject(reply);
		return false;
	}
	if (reply->type != REDIS_REPLY_NIL)
	{
		freeReplyObject(reply);
		SendResult(fd, REGISTER, IM_ERROR, "user has exist!");
		return false;
	}
	freeReplyObject(reply);
	char sqlCmd[100] = { 0 };
	sprintf(sqlCmd, "select id from user where id = %d", id);
	if (mysql_real_query(mysql->mpcon, sqlCmd, strlen(sqlCmd)))
	{
		LOGE("mysql select fail!");
		SendResult(fd, REGISTER, IM_ERROR, "server error, register fail!");
		return false;
	}
	MYSQL_RES *mp_res;
	MYSQL_ROW mp_row;
	mp_res = mysql_store_result(mysql->mpcon);
	mp_row = mysql_fetch_row(mp_res);

	if (mp_row != 0)
	{
		SendResult(fd, REGISTER, IM_ERROR, "user has exist!");
		return false;
	}
	
	sprintf(sqlCmd, "insert into user value (%d, \"%s\", \"%s\", %d)", id, name.c_str(), passwd.c_str(), type);
	if (mysql_real_query(mysql->mpcon, sqlCmd, strlen(sqlCmd)))
	{
		LOGE("mysql insert fail!");
		SendResult(fd, REGISTER, IM_ERROR, "server error, register fail!");
		return false;
	}
	
	SendResult(fd, REGISTER, IM_OK, "user register success!");
	return true;
}

bool Login(int fd, std::string ReqMsg)
{
	PDUHEAD* pReqHeader = (PDUHEAD*)ReqMsg.c_str();
	std::string req_body = pReqHeader->GetBody();
	IM::User::Msg::IMLoginReq user;
	user.ParseFromString(req_body);
	int id = user.id();
	std::string passwd = user.passwd();
	int type = user.type();

	char redisCmd[100] = { 0 };
	sprintf(redisCmd, "hgetall user:id:%d", id);
	redisReply* reply;
	if (!redis->RedisCommand(redisCmd, reply))
	{
		LOGE("redis command fail!");
		SendResult(fd, REGISTER, IM_ERROR, "server error, login fail!");
		freeReplyObject(reply);
		return false;
	}
	if (reply->type != REDIS_REPLY_NIL)
	{
		int __id = atoi(reply->element[1]->str);
		std::string pw = reply->element[5]->str;
		freeReplyObject(reply);
		if (__id == id && pw == passwd)
		{
			SendResult(fd, IM_OK, LOGIN, "login success!");
			return true;
		}
		else
		{
			SendResult(fd, IM_ERROR, LOGIN, "passwd error, login fail!");
			return false;
		}
	}
	freeReplyObject(reply);
	//=============================================

	//char sqlCmd[100] = { 0 };
	//sprintf(sqlCmd, "select id from user where id = %d", id);
	//if (mysql_real_query(mysql->mpcon, sqlCmd, strlen(sqlCmd)))
	//{
	//	LOGE("mysql select fail!");
	//	SendResult(fd, REGISTER, IM_ERROR, "server error, register fail!");
	//	return false;
	//}
	//MYSQL_RES* mp_res;
	//MYSQL_ROW mp_row;
	//mp_res = mysql_store_result(mysql->mpcon);
	//mp_row = mysql_fetch_row(mp_res);

	//if (mp_row != 0)
	//{
	//	SendResult(fd, REGISTER, IM_ERROR, "user has exist!");
	//	return false;
	//}

	////sprintf(sqlCmd, "insert into user value (%d, \"%s\", \"%s\", %d)", id, name.c_str(), passwd.c_str(), type);
	//if (mysql_real_query(mysql->mpcon, sqlCmd, strlen(sqlCmd)))
	//{
	//	LOGE("mysql insert fail!");
	//	SendResult(fd, REGISTER, IM_ERROR, "server error, register fail!");
	//	return false;
	//}

	//SendResult(fd, REGISTER, IM_OK, "user register success!");
	//return true;
}


