#include "control.h"

extern int _epollfd;
extern std::map<int, struct sockaddr_in> _cliInfo;

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
		epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd, NULL);
		close(fd);
		//LOGI("***************ip: %s, port: %d exit!", inet_ntoa(_cliInfo[fd].sin_addr), ntohs(_cliInfo[fd].sin_port));
		_cliInfo.erase(fd);
		//std::cout<<SerSocket::GetSerSocket()->GetErrMsg() << std::endl;
	}
}

Control::Control()
{
	_model.insert(std::make_pair(REGISTER, Register));
	_model.insert(std::make_pair(LOGIN, Login));
	_model.insert(std::make_pair(SELECT, SelectQues));
	_model.insert(std::make_pair(INSERT, InsertQues));
	_model.insert(std::make_pair(DELETE, DelQues));
	_model.insert(std::make_pair(GETALL, GetAllQues));

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

	std::cout<<"[Register]--> id: "<<id<<" name: "<<name<<" passwd: "<<passwd<<" type: "<<type<<std::endl;
	LOGE("---id: %d, name: %s, passwd: %s, type: %d", id, name.c_str(), passwd.c_str(), type);
	char redisCmd[100] = { 0 };
	sprintf(redisCmd, "hgetall user:id:%d", id);

	std::map<std::string, std::string> Redisresult;
	if (!RedisPool::GetRedisPool()->Query(redisCmd, Redisresult))
	{
		LOGE("redis command fail!");
		SendResult(fd, REGISTER, IM_ERROR, "服务器出错，注册失败");
		return false;
	}
	
	if (Redisresult.size() != 0)
	{
		LOGE("redis get data!");
		SendResult(fd, REGISTER, IM_ERROR, "用户已经存在，注册失败");
		return true;
	}

	char sqlCmd[100] = { 0 };
	sprintf(sqlCmd, "select * from user where id = %d", id);

	std::map<std::string, std::vector<std::string>> result;
	bool flag = MysqlPool::GetMysqlPool()->Query(sqlCmd, result);
	if (!flag)
	{
		LOGE("mysql get data!");
		LOGE("mysql select fail!");
		SendResult(fd, REGISTER, IM_ERROR, "服务器出错，注册失败");
		return false;
	}

	if (result.begin()->second.size() != 0)
	{
		SendResult(fd, REGISTER, IM_ERROR, "用户已经存在，注册失败");
		return true;
	}
	
	sprintf(sqlCmd, "insert into user value (%d, \"%s\", \"%s\", %d)", id, name.c_str(), passwd.c_str(), type);
	if (!MysqlPool::GetMysqlPool()->Insert(sqlCmd))
	{
		LOGE("mysql insert fail!");
		SendResult(fd, REGISTER, IM_ERROR, "服务器出错，注册失败");
		return false;
	}
	
	LOGE("mysql get data!");
	SendResult(fd, REGISTER, IM_OK, "注册成功");
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
	std::cout << "[Login]--> id: " << id << " passwd: " << passwd << " type: " << type << std::endl;
	char redisCmd[100] = { 0 };
	sprintf(redisCmd, "hgetall user:id:%d", id);

	std::map<std::string, std::string> Redisresult;
	if (!RedisPool::GetRedisPool()->Query(redisCmd, Redisresult))
	{
		LOGE("redis command fail!");
		SendResult(fd, LOGIN, IM_ERROR, "服务器出错，登录失败");
		return false;
	}

	if (Redisresult.size() != 0)
	{
		int __id = atoi(Redisresult["id"].c_str());
		std::string pw = Redisresult["passwd"];
		int __type = atoi(Redisresult["type"].c_str());
		if (__id == id && pw == passwd && type == __type)
		{
			LOGE("redis get data!");
			SendResult(fd, LOGIN, IM_OK, "登录成功");
			return true;
		}
		else
		{
			SendResult(fd, LOGIN, IM_ERROR, "密码错误，登录失败");
			return true;
		}
	}

	char sqlCmd[100] = { 0 };
	sprintf(sqlCmd, "select id, name ,passwd, type from user where id = %d", id);
	std::map<std::string, std::vector<std::string>> result;
	bool flag = MysqlPool::GetMysqlPool()->Query(sqlCmd, result);
	if (!flag)
	{
		LOGE("mysql select fail!");
		SendResult(fd, LOGIN, IM_ERROR, "服务器出错，登录失败");
		return false;
	}

	if (result.begin()->second.size() == 0)
	{
		SendResult(fd, LOGIN, IM_ERROR, "用户不存在，登录失败");
		return true;
	}

	if (atoi(result["id"][0].c_str()) == id && passwd == result["passwd"][0] && type == atoi(result["type"][0].c_str()))
	{
		SendResult(fd, LOGIN, IM_OK, "登录成功");
		sprintf(redisCmd, "hmset user:id:%d id %d name %s passwd %s type %d", id, id, result["name"][0].c_str(), passwd.c_str(), type);
		if (!RedisPool::GetRedisPool()->Insert(redisCmd))
		{
			LOGE("redis command fail!");
			return false;
		}
		return true;
	}
	
	LOGE("mysql get data!")
	SendResult(fd, LOGIN, IM_ERROR, "密码错误，登录失败");
	LOGE("bool Login(int fd, std::string ReqMsg)-->end");
	return true;
}

bool SelectQues(int fd, std::string ReqMsg)
{
	return false;
}

bool InsertQues(int fd, std::string ReqMsg)
{
	PDUHEAD* pReqHeader = (PDUHEAD*)ReqMsg.c_str();
	std::string req_body = pReqHeader->GetBody();
	IM::User::Msg::IMInsertMsg inse;
	inse.ParseFromString(req_body);
	int type = inse.type();
	std::string msg = inse.msg();
	int degree = inse.degree();

	
	std::string key;
	if (type == LIST)
		key = "list:" + std::to_string(degree);
	else if (type == STRING)
		key = "string:" + std::to_string(degree);
	else if (type == STACK)
		key = "stack:" + std::to_string(degree);
	else if (type == QUEUE)
		key = "queue:" + std::to_string(degree);
	else if (type == TREE)
		key = "tree:" + std::to_string(degree);

	std::string field = key+"*id:" + std::to_string(rand() % 9999 + 1);
	std::string cmd = "exists " + key;
	int flag = RedisPool::GetRedisPool()->Exist(cmd.data());
	if (flag == -1)
	{
		LOGI("redis command fail!");
		SendResult(fd, INSERT, IM_OK, "服务器出错，插入失败");
		return false;
	}
	else if (flag == 1)
	{
		cmd.clear();
		cmd = "hexists " + key + " " + field;
		flag = RedisPool::GetRedisPool()->Exist(cmd.data());
		while (flag == 1)
		{
			field.clear();
			field = key+"*id:" + std::to_string(rand() % 9999 + 1);
			flag = RedisPool::GetRedisPool()->Exist(cmd.data());
		}
		if (flag == -1)
		{
			LOGI("redis command fail!");
			SendResult(fd, INSERT, IM_OK, "服务器出错，插入失败");
			return false;
		}
	}

	std::string redisCmd;
	redisCmd = "HSET " + key + " " + field + " " + msg;
	if (!RedisPool::GetRedisPool()->Insert(redisCmd.data()))
	{
		LOGI("redis command fail!");
		SendResult(fd, INSERT, IM_OK, "服务器出错，插入失败");
		return false;
	}
	SendResult(fd, INSERT, IM_OK, "插入成功");
	return true;
}

bool DelQues(int fd, std::string ReqMsg)
{
	PDUHEAD* pReqHeader = (PDUHEAD*)ReqMsg.c_str();
	std::string req_body = pReqHeader->GetBody();
	IM::User::Msg::IMRspMsg req;
	req.ParseFromString(req_body);
	std::string field = req.msg();
	std::string key = field.substr(0, field.find('*'));
	std::string redisCmd = "hdel " + key + " " + field;

	int flag = RedisPool::GetRedisPool()->Del(redisCmd.data());
	if (flag)
	{
		SendResult(fd, GETALL, IM_OK, "删除成功");
		return true;
	}
	else
	{
		SendResult(fd, GETALL, IM_OK, "删除失败");
		return false;
	}
}

bool GetAllQues(int fd, std::string ReqMsg)
{
	std::string rsp = "\n";

	//处理string
	std::string key;
	std::string redisCmd;
	for (int i = 1; i < 6; ++i)
	{
		redisCmd.clear();
		key.clear();
		key = "string:" + std::to_string(i);
		redisCmd = "exists " + key;
		int flag = RedisPool::GetRedisPool()->Exist(redisCmd.data());
		if (flag == -1)
		{
			LOGI("redis command fail!");
			SendResult(fd, GETALL, IM_OK, "服务器出错，获取失败");
			return false;
		}
		else if (flag == 0)
		{
			continue;
		}
		else
		{
			redisCmd.clear();
			redisCmd = "hgetall " + key;
			std::map<std::string, std::string> results;
			bool ret = RedisPool::GetRedisPool()->Query(redisCmd.data(), results);
			if (!ret)
			{
				LOGI("redis command fail!");
				SendResult(fd, GETALL, IM_OK, "服务器出错，获取失败");
				return false;
			}
			for (auto it : results)
			{
				std::string str = it.first + "[==]" + it.second;
				rsp = rsp + str + "\n";
				str.clear();
			}
		}
	}

	for (int i = 1; i < 6; ++i)
	{
		redisCmd.clear();
		key.clear();
		key = "list:"+std::to_string(i);
		redisCmd = "exists " + key;
		int flag = RedisPool::GetRedisPool()->Exist(redisCmd.data());
		if (flag == -1)
		{
			LOGI("redis command fail!");
			SendResult(fd, GETALL, IM_OK, "服务器出错，获取失败");
			return false;
		}
		else if (flag == 0)
		{
			continue;
		}
		else
		{
			redisCmd.clear();
			redisCmd = "hgetall " + key;
			std::map<std::string, std::string> results;
			bool ret = RedisPool::GetRedisPool()->Query(redisCmd.data(), results);
			if (!ret)
			{
				LOGI("redis command fail!");
				SendResult(fd, GETALL, IM_OK, "服务器出错，获取失败");
				return false;
			}
			for (auto it : results)
			{
				std::string str = it.first + "[==]" + it.second;
				rsp = rsp + str + "\n";
				str.clear();
			}
		}
	}

	for (int i = 1; i < 6; ++i)
	{
		redisCmd.clear();
		key.clear();
		key = "stack:" + std::to_string(i);
		redisCmd = "exists " + key;
		int flag = RedisPool::GetRedisPool()->Exist(redisCmd.data());
		if (flag == -1)
		{
			LOGI("redis command fail!");
			SendResult(fd, GETALL, IM_OK, "服务器出错，获取失败");
			return false;
		}
		else if (flag == 0)
		{
			continue;
		}
		else
		{
			redisCmd.clear();
			redisCmd = "hgetall " + key;
			std::map<std::string, std::string> results;
			bool ret = RedisPool::GetRedisPool()->Query(redisCmd.data(), results);
			if (!ret)
			{
				LOGI("redis command fail!");
				SendResult(fd, GETALL, IM_OK, "服务器出错，获取失败");
				return false;
			}
			for (auto it : results)
			{
				std::string str = it.first + "[==]" + it.second;
				rsp = rsp + str + "\n";
				str.clear();
			}
		}
	}

	for (int i = 1; i < 6; ++i)
	{
		redisCmd.clear();
		key.clear();
		key = "queue:" + std::to_string(i);
		redisCmd = "exists " + key;
		int flag = RedisPool::GetRedisPool()->Exist(redisCmd.data());
		if (flag == -1)
		{
			LOGI("redis command fail!");
			SendResult(fd, GETALL, IM_OK, "服务器出错，获取失败");
			return false;
		}
		else if (flag == 0)
		{
			continue;
		}
		else
		{
			redisCmd.clear();
			redisCmd = "hgetall " + key;
			std::map<std::string, std::string> results;
			bool ret = RedisPool::GetRedisPool()->Query(redisCmd.data(), results);
			if (!ret)
			{
				LOGI("redis command fail!");
				SendResult(fd, GETALL, IM_OK, "服务器出错，获取失败");
				return false;
			}
			for (auto it : results)
			{
				std::string str = it.first + "[==]" + it.second;
				rsp = rsp + str + "\n";
				str.clear();
			}
		}
	}

	for (int i = 1; i < 6; ++i)
	{
		redisCmd.clear();
		key.clear();
		key = "tree:" + std::to_string(i);
		redisCmd = "exists " + key;
		int flag = RedisPool::GetRedisPool()->Exist(redisCmd.data());
		if (flag == -1)
		{
			LOGI("redis command fail!");
			SendResult(fd, GETALL, IM_OK, "服务器出错，获取失败");
			return false;
		}
		else if (flag == 0)
		{
			continue;
		}
		else
		{
			redisCmd.clear();
			redisCmd = "hgetall " + key;
			std::map<std::string, std::string> results;
			bool ret = RedisPool::GetRedisPool()->Query(redisCmd.data(), results);
			if (!ret)
			{
				LOGI("redis command fail!");
				SendResult(fd, GETALL, IM_OK, "服务器出错，获取失败");
				return false;
			}
			for (auto it : results)
			{
				std::string str = it.first + "[==]" + it.second;
				rsp = rsp + str + "\n";
				str.clear();
			}
		}
	}

	SendResult(fd, GETALL, IM_OK, rsp);
	return true;
}


