#include "view.h"

static void trim(std::string& str)
{
	int index = 0;
	if (!str.empty())
	{
		while ((index = str.find(' ', index)) != std::string::npos)
		{
			str.erase(index, 1);
		}
	}
}

int Register::Process()
{
	int id = 0;
	std::string name;
	std::string passwd;
	int type;
	std::cout << "---> 请输入ID: ";
	std::cin >> id;
	while (std::cin.fail())
	{
		std::string str;
		std::cin.clear();
		std::cin >> str;
		std::cin.ignore();

		std::cout << "\033[31m[提  示]: 输入ID错误\033[0m" << std::endl;
		std::cout << "---> 请再次输入ID: ";
		std::cin >> id;
	}

	std::cout << "---> 请输入姓名: ";
	std::cin >> name;
	std::cout << "---> 请输入密码: ";
	std::cin >> passwd;
	std::cout << "\033[36m*------- 类型: 学生 1--老师 2 -------*\033[0m" << std::endl;

	std::cout << "---> 请选择类型: ";
	std::cin >> type;
	while (std::cin.fail())
	{
		std::string str;
		std::cin.clear();
		std::cin >> str;
		std::cin.ignore();
		std::cout << "\033[31m[提  示]: 输入类型错误\033[0m" << std::endl;
		std::cout << "---> 请再次输入类型: ";
		std::cin >> type;
	}

	if (type != STUDENT && type != ADMIN)
	{
		std::cout << "\033[31m[提  示]: 输入类型错误\033[0m" << std::endl;
		return IM_FALSE;
	}

	PDUHEAD pReqHeader;
	pReqHeader.SetCmd(REGISTER);
	char ReqBuffer[1024 * 1024] = { 0 };
	IM::User::Msg::IMRegisterReq Msg;
	Msg.set_id(id);
	Msg.set_name(name.c_str());
	Msg.set_passwd(passwd.c_str());
	Msg.set_type(type);
	std::string ReqMsg;
	Msg.SerializeToString(&ReqMsg);
	memcpy(ReqBuffer, (void*)&pReqHeader, pReqHeader.GetHeadLen());
	memcpy(ReqBuffer + pReqHeader.GetHeadLen(), ReqMsg.c_str(), ReqMsg.size());
	((PDUHEAD*)ReqBuffer)->SetPackLen(pReqHeader.GetHeadLen() + ReqMsg.size());

	//send(fd, ReqBuffer, pReqHeader.GetHeadLen() + ReqMsg.size(), 0);
	int res = CliSocket::GetCliSocket()->SendBuf(ReqBuffer, pReqHeader.GetHeadLen() + ReqMsg.size());
	if (res != SOCK_SUC)
	{
		std::cout << "\033[34m[提  示]: 服务器掉线\033[0m" << std::endl;
		sleep(1);
		exit(0);
		//std::cout << CliSocket::GetCliSocket()->GetErrMsg() << std::endl;
		//return IM_FALSE;
	}

	char RspBuffer[1024 * 1024] = { 0 };
	//recv(fd, RspBuffer, 1024 * 1024, 0);

	int len;
	res = CliSocket::GetCliSocket()->RecvBuf(RspBuffer, 1024 * 1024, len);
	if (res != SOCK_SUC)
	{
		std::cout << "\033[34m[提  示]: 服务器掉线\033[0m" << std::endl;
		sleep(1);
		exit(0);
		/*std::cout << CliSocket::GetCliSocket()->GetErrMsg() << std::endl;
		return IM_FALSE;*/
	}

	IM::User::Msg::IMRspMsg Msg2;
	PDUHEAD* pRspHeader = (PDUHEAD*)RspBuffer;
	std::string RspMsg = pRspHeader->GetBody();
	Msg2.ParseFromString(RspMsg);
	int ret = Msg2.ret();
	std::string msg = Msg2.msg();
	std::cout << "\033[34m[提  示]: " << msg << "\033[0m" << std::endl;
	if (ret == IM_OK)
	{
		return IM_TRUE;
	}
	else
	{
		return IM_FALSE;
	}
}

int Login::Process()
{
	int id = 0;
	std::string name;
	std::string passwd;
	int type;
	std::cout << "---> 请输入ID: ";
	std::cin >> id;
	while (std::cin.fail())
	{
		std::string str;
		std::cin.clear();
		std::cin >> str;
		std::cin.ignore();
		std::cout << "\033[31m[提  示]: 输入ID错误\033[0m" << std::endl;
		std::cout << "---> 请再次输入ID: ";
		std::cin >> id;
	}

	std::cout << "---> 请输入密码: ";
	std::cin >> passwd;
	std::cout << "\033[36m*------- 类型: 学生 1--老师 2 -------*\033[0m" << std::endl;

	std::cout << "---> 请选择类型: ";
	std::cin >> type;
	while (std::cin.fail())
	{
		std::string str;
		std::cin.clear();
		std::cin >> str;
		std::cin.ignore();
		std::cout << "\033[31m[提  示]: 输入类型错误\033[0m" << std::endl;
		std::cout << "---> 请再次输入类型: ";
		std::cin >> type;
	}
	if (type != STUDENT && type != ADMIN)
	{
		std::cout << "\033[31m[提  示]: 输入类型错误\033[0m" << std::endl;
		return IM_FALSE;
	}

	PDUHEAD pReqHeader;
	pReqHeader.SetCmd(LOGIN);
	char ReqBuffer[1024 * 1024] = { 0 };
	IM::User::Msg::IMLoginReq Msg;
	Msg.set_id(id);
	Msg.set_passwd(passwd.c_str());
	Msg.set_type(type);
	std::string ReqMsg;
	Msg.SerializeToString(&ReqMsg);
	memcpy(ReqBuffer, (void*)&pReqHeader, pReqHeader.GetHeadLen());
	memcpy(ReqBuffer + pReqHeader.GetHeadLen(), ReqMsg.c_str(), ReqMsg.size());
	((PDUHEAD*)ReqBuffer)->SetPackLen(pReqHeader.GetHeadLen() + ReqMsg.size());

	//send(fd, ReqBuffer, pReqHeader.GetHeadLen() + ReqMsg.size(), 0);
	int res = CliSocket::GetCliSocket()->SendBuf(ReqBuffer, pReqHeader.GetHeadLen() + ReqMsg.size());
	if (res != SOCK_SUC)
	{
		std::cout << "\033[34m[提  示]: 服务器掉线\033[0m" << std::endl;
		sleep(1);
		exit(0);
		/*std::cout << CliSocket::GetCliSocket()->GetErrMsg() << std::endl;
		return IM_FALSE;*/
	}

	char RspBuffer[1024 * 1024] = { 0 };
	//recv(fd, RspBuffer, 1024 * 1024, 0);
	int len;
	res = CliSocket::GetCliSocket()->RecvBuf(RspBuffer, 1024 * 1024, len);
	if (res != SOCK_SUC)
	{
		std::cout << "\033[34m[提  示]: 服务器掉线\033[0m" << std::endl;
		sleep(1);
		exit(0);
		/*std::cout << CliSocket::GetCliSocket()->GetErrMsg() << std::endl;
		return IM_FALSE;*/
	}

	IM::User::Msg::IMRspMsg Msg2;
	PDUHEAD* pRspHeader = (PDUHEAD*)RspBuffer;
	std::string RspMsg = pRspHeader->GetBody();
	Msg2.ParseFromString(RspMsg);
	int ret = Msg2.ret();
	std::string msg = Msg2.msg();
	std::cout << "\033[34m[提  示]: " << msg << "\033[0m" << std::endl;
	if (ret == IM_OK)
	{
		if (type == ADMIN)
		{
			return ADMIN;
		}
		else if (type == STUDENT)
		{
			return STUDENT;
		}
	}
	else
	{
		return IM_FALSE;
	}
}

int SelectQues::Process()
{
	PDUHEAD pReqHeader;
	pReqHeader.SetCmd(SELECT);
	char ReqBuffer[1024 * 1024] = { 0 };
	memcpy(ReqBuffer, (void*)&pReqHeader, pReqHeader.GetHeadLen());
	((PDUHEAD*)ReqBuffer)->SetPackLen(pReqHeader.GetHeadLen());
	int res = CliSocket::GetCliSocket()->SendBuf(ReqBuffer, pReqHeader.GetHeadLen());
	if (res != SOCK_SUC)
	{
		std::cout << "\033[34m[提  示]: 服务器掉线\033[0m" << std::endl;
		sleep(1);
		exit(0);
	}


	char RspBuffer[1024 * 1024] = { 0 };
	int len;
	res = CliSocket::GetCliSocket()->RecvBuf(RspBuffer, 1024 * 1024, len);
	if (res != SOCK_SUC)
	{
		std::cout << "\033[34m[提  示]: 服务器掉线\033[0m" << std::endl;
		sleep(1);
		exit(0);
	}

	IM::User::Msg::IMRspMsg Msg2;
	PDUHEAD* pRspHeader = (PDUHEAD*)RspBuffer;
	std::string RspMsg = pRspHeader->GetBody();
	Msg2.ParseFromString(RspMsg);
	int ret = Msg2.ret();
	std::string msg = Msg2.msg();
	if (ret <= 0)
	{
		std::cout << "\033[34m[提  示]: " << msg << "\033[0m" << std::endl;
		return IM_FALSE;
	}
	else
	{
		std::cout << "\033[32m试题个数: " << ret << "\033[0m" << std::endl;
		std::cout << "\033[32m" << msg << "\033[0m" << std::endl;
		return IM_OK;
	}
}

int InsertQues::Process()
{
	int errnum = 0;   //输入次数
	int degree = 0;
	int type = 0;
	std::string str;

	std::cout << "\033[36m*-- String 1--List 2--Stack 3--Queue 4--Tree 5 --*\033[0m" << std::endl;
	std::cout << "---> 请选择类型: ";
	std::cin >> type;
	while (std::cin.fail())
	{
		errnum++;
		if (errnum >= 3)
		{
			std::cout << "\033[31m[提  示]: 输入次数已上线\033[0m" << std::endl;
			return IM_FALSE;
		}
		std::string str;
		std::cin.clear();
		std::cin >> str;
		std::cin.ignore();

		std::cout << "\033[31m[提  示]: 输入类型错误\033[0m" << std::endl;
		std::cout << "---> 请再次输入: ";
		std::cin >> type;
	}
	while (type <=0 || type >5)
	{
		errnum++;
		if (errnum >= 3)
		{
			std::cout << "\033[31m[提  示]: 输入次数已上线\033[0m" << std::endl;
			return IM_FALSE;
		}
		std::cout << "\033[31m[提  示]: 输入类型错误\033[0m" << std::endl;
		std::cout << "---> 请再次输入: ";
		std::cin >> type;
		while (std::cin.fail())
		{
			errnum++;
			if (errnum >= 3)
			{
				std::cout << "\033[31m[提  示]: 输入次数已上线\033[0m" << std::endl;
				return IM_FALSE;
			}
			std::string str;
			std::cin.clear();
			std::cin >> str;
			std::cin.ignore();

			std::cout << "\033[31m[提  示]: 输入类型错误\033[0m" << std::endl;
			std::cout << "---> 请再次输入: ";
			std::cin >> type;
		}
	}
	errnum = 0;
	std::cout << "---> 请输入难度系数[1--5]: ";
	std::cin >> degree;
	while (std::cin.fail())
	{
		errnum++;
		if (errnum >= 3)
		{
			std::cout << "\033[31m[提  示]: 输入次数已上线\033[0m" << std::endl;
			return IM_FALSE;
		}
		std::string str;
		std::cin.clear();
		std::cin >> str;
		std::cin.ignore();

		std::cout << "\033[31m[提  示]: 输入类型错误\033[0m" << std::endl;
		std::cout << "---> 请再次输入: ";
		std::cin >> degree;
	}
	while (degree <= 0 || degree > 5)
	{
		errnum++;
		if (errnum >= 3)
		{
			std::cout << "\033[31m[提  示]: 输入次数已上线\033[0m" << std::endl;
			return IM_FALSE;
		}
		std::cout << "\033[31m[提  示]: 输入类型错误\033[0m" << std::endl;
		std::cout << "---> 请再次输入: ";
		std::cin >> degree;
		while (std::cin.fail())
		{
			errnum++;
			if (errnum >= 3)
			{
				std::cout << "\033[31m[提  示]: 输入次数已上线\033[0m" << std::endl;
				return IM_FALSE;
			}
			std::string str;
			std::cin.clear();
			std::cin >> str;
			std::cin.ignore();

			std::cout << "\033[31m[提  示]: 输入类型错误\033[0m" << std::endl;
			std::cout << "---> 请再次输入: ";
			std::cin >> degree;
		}
	}

	std::cout << "---> 请输入试题内容: ";
	std::cin.clear();
	std::cin.ignore();
	getline(std::cin, str);
	trim(str);
	/*std::cin >> str;

	std::cout << str << std::endl;*/
	PDUHEAD pReqHeader;
	pReqHeader.SetCmd(INSERT);
	char ReqBuffer[1024 * 1024] = { 0 };
	IM::User::Msg::IMInsertMsg Msg;
	Msg.set_degree(degree);
	Msg.set_type(type+30);
	Msg.set_msg(str);
	std::string ReqMsg;
	Msg.SerializeToString(&ReqMsg);
	memcpy(ReqBuffer, (void*)&pReqHeader, pReqHeader.GetHeadLen());
	memcpy(ReqBuffer + pReqHeader.GetHeadLen(), ReqMsg.c_str(), ReqMsg.size());
	((PDUHEAD*)ReqBuffer)->SetPackLen(pReqHeader.GetHeadLen() + ReqMsg.size());
	int res = CliSocket::GetCliSocket()->SendBuf(ReqBuffer, pReqHeader.GetHeadLen() + ReqMsg.size());
	if (res != SOCK_SUC)
	{
		std::cout << "\033[34m[提  示]: 服务器掉线\033[0m" << std::endl;
		sleep(1);
		exit(0);
	}

	char RspBuffer[1024 * 1024] = { 0 };
	int len;
	res = CliSocket::GetCliSocket()->RecvBuf(RspBuffer, 1024 * 1024, len);
	if (res != SOCK_SUC)
	{
		std::cout << "\033[34m[提  示]: 服务器掉线\033[0m" << std::endl;
		sleep(1);
		exit(0);
	}

	IM::User::Msg::IMRspMsg Msg2;
	PDUHEAD* pRspHeader = (PDUHEAD*)RspBuffer;
	std::string RspMsg = pRspHeader->GetBody();
	Msg2.ParseFromString(RspMsg);
	int ret = Msg2.ret();
	std::string msg = Msg2.msg();
	std::cout << "\033[34m[提  示]: " << msg << "\033[0m" << std::endl;
	if (ret == IM_OK)
	{
		return IM_TRUE;
	}
	else
	{
		return IM_FALSE;
	}
}

int GetAllQues::Process()
{

	return 0;
}

int DelQues::Process()
{
	return 0;
}
