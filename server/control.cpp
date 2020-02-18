#include "control.h"

Redis* redis = Redis::GetRedis();

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
	int res = SerSocket::GetSerSocket()->SendBuf(fd, RspBuffer, pReqHeader.GetHeadLen() + RspMsg.size());
	if (res != SOCK_SUC)
	{
		std::cout<<SerSocket::GetSerSocket()->GetErrMsg() << std::endl;
	}
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
	LOGE("bool Register(int fd, std::string ReqMsg)");
	PDUHEAD* pReqHeader = (PDUHEAD*)ReqMsg.c_str();
	std::string req_body = pReqHeader->GetBody();
	IM::User::Msg::IMRegisterReq user;
	user.ParseFromString(req_body);
	int id = user.id();
	std::string name = user.name();
	std::string passwd = user.passwd();
	int type = user.type();
	LOGE("---id: %d, name: %s, passwd: %s, type: %d", id, name.c_str(), passwd.c_str(), type);
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
	if (reply->type != REDIS_REPLY_NIL && reply->elements != 0)
	{
		LOGE("redis get data!");
		freeReplyObject(reply);
		SendResult(fd, REGISTER, IM_ERROR, "user has exist!");
		return false;
	}
	freeReplyObject(reply);
	char sqlCmd[100] = { 0 };
	sprintf(sqlCmd, "select * from user where id = %d", id);

	std::map<std::string, std::vector<std::string>> result;
	bool flag = MysqlPool::GetMysqlPool()->Query(sqlCmd, result);
	if (!flag)
	{
		LOGE("mysql get data!");
		LOGE("mysql select fail!");
		SendResult(fd, REGISTER, IM_ERROR, "server error, register fail!");
		return false;
	}

	std::cout << "*****************" << result.size() << std::endl;
	std::cout << "*****************" << result.begin()->second.size() << std::endl;
	for (auto it : result)
	{
		std::cout << it.first << "  ";
		for (auto it2 : it.second)
		{
			std::cout << it2 << "  ";
		}
		std::cout<<std::endl;
	}
	std::cout << std::endl;


	if (result.begin()->second.size() != 0)
	{
		SendResult(fd, REGISTER, IM_ERROR, "user has exist!");
		return false;
	}
	
	sprintf(sqlCmd, "insert into user value (%d, \"%s\", \"%s\", %d)", id, name.c_str(), passwd.c_str(), type);
	if (!MysqlPool::GetMysqlPool()->Insert(sqlCmd))
	{
		LOGE("mysql insert fail!");
		SendResult(fd, REGISTER, IM_ERROR, "server error, register fail!");
		return false;
	}
	
	LOGE("mysql get data!");
	SendResult(fd, REGISTER, IM_OK, "register success!");
	LOGE("bool Register(int fd, std::string ReqMsg)-->end");
	return true;
}

bool Login(int fd, std::string ReqMsg)
{
	LOGE("bool Login(int fd, std::string ReqMsg)");
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
		SendResult(fd, LOGIN, IM_ERROR, "server error, login fail!");
		freeReplyObject(reply);
		return false;
	}

	if (reply->type != REDIS_REPLY_NIL && reply->elements >= 8)
	{
		int __id = atoi(reply->element[1]->str);
		std::string pw = reply->element[5]->str;
		int __type = atoi(reply->element[7]->str);
		freeReplyObject(reply);
		if (__id == id && pw == passwd && type == __type)
		{
			LOGE("redis get data!");
			SendResult(fd, IM_OK, IM_OK, "login success!");
			return true;
		}
		else
		{
			SendResult(fd, LOGIN, LOGIN, "passwd error, login fail!");
			return false;
		}
	}
	freeReplyObject(reply);

	char sqlCmd[100] = { 0 };
	sprintf(sqlCmd, "select id, name ,passwd, type from user where id = %d", id);
	std::map<std::string, std::vector<std::string>> result;
	bool flag = MysqlPool::GetMysqlPool()->Query(sqlCmd, result);
	if (!flag)
	{
		LOGE("mysql select fail!");
		SendResult(fd, LOGIN, IM_ERROR, "server error, register fail!");
		return false;
	}

	if (result.begin()->second.size() == 0)
	{
		SendResult(fd, LOGIN, IM_ERROR, "user has not exist!");
		return false;
	}

	if (atoi(result["id"][0].c_str()) == id && passwd == result["passwd"][0] && type == atoi(result["type"][0].c_str()))
	{
		SendResult(fd, LOGIN, IM_OK, "login success!");
		sprintf(redisCmd, "hmset user:id:%d id %d name %s passwd %s type %d", id, id, result["name"][0].c_str(), passwd.c_str(), type);
		if (!redis->RedisCommand(redisCmd, reply))
		{
			LOGE("redis command fail!");
			freeReplyObject(reply);
			return false;
		}
		return true;
	}
	
	LOGE("mysql get data!")
	SendResult(fd, LOGIN, IM_ERROR, "passwd error, login fail!");
	LOGE("bool Login(int fd, std::string ReqMsg)-->end");
	return false;
}


