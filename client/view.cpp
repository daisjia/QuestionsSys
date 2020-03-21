#include "view.h"

int Register::Process()
{
	int id = 0;
	std::string name;
	std::string passwd;
	int type;
	std::cout << "---> please input id: ";
	std::cin >> id;
	while(std::cin.fail())
	{
		std::string str;
		std::cin.clear();
		std::cin >> str;
		std::cin.ignore();
		
		std::cout << "\033[31m===> input error!\033[0m" << std::endl;
		std::cout << "please again input id: ";
		std::cin >> id;
	}

	std::cout << "---> please input name: ";
	std::cin >> name;
	std::cout << "---> please input passwd: ";
	std::cin >> passwd;
	std::cout << "---> type: student 0 && admin 1 "<<std::endl;
	std::cout << "---> please input type: ";
	std::cin >> type;
	while (std::cin.fail())
	{
		std::string str;
		std::cin.clear();
		std::cin >> str;
		std::cin.ignore();
		std::cout << "\033[31m===> input error!\033[0m" << std::endl;
		std::cout << "please again input type: ";
		std::cin >> type;
	}

	if (type != STUDENT && type != ADMIN)
	{
		std::cout << "\033[31m===> type error!\033[0m" << std::endl;
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
		std::cout << CliSocket::GetCliSocket()->GetErrMsg() << std::endl;
		return IM_FALSE;
	}

	char RspBuffer[1024 * 1024] = { 0 };
	//recv(fd, RspBuffer, 1024 * 1024, 0);
	
	int len;
	res = CliSocket::GetCliSocket()->RecvBuf(RspBuffer, 1024 * 1024, len);
	if (res != SOCK_SUC)
	{
		std::cout << CliSocket::GetCliSocket()->GetErrMsg() << std::endl;
		return IM_FALSE;
	}

	IM::User::Msg::IMRspMsg Msg2;
	PDUHEAD* pRspHeader = (PDUHEAD*)RspBuffer;
	std::string RspMsg = pRspHeader->GetBody();
	Msg2.ParseFromString(RspMsg);
	int ret = Msg2.ret();
	std::string msg = Msg2.msg();
	std::cout << "\033[34m------->SERVER: " << msg << "\033[0m" << std::endl;
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
	std::cout << "---> please input id: ";
	std::cin >> id;
	while (std::cin.fail())
	{
		std::string str;
		std::cin.clear();
		std::cin >> str;
		std::cin.ignore();
		std::cout << "\033[31m===> input error!\033[0m" << std::endl;
		std::cout << "please again input id: ";
		std::cin >> id;
	}

	std::cout << "---> please input passwd: ";
	std::cin >> passwd;
	std::cout << "---> type: student 0 && admin 1 " << std::endl;
	std::cout << "---> please input type: ";
	std::cin >> type;
	while (std::cin.fail())
	{
		std::string str;
		std::cin.clear();
		std::cin >> str;
		std::cin.ignore();
		std::cout << "\033[31m===> input error!\033[0m" << std::endl;
		std::cout << "please again input type: ";
		std::cin >> type;
	}
	if (type != STUDENT && type != ADMIN)
	{
		std::cout << "\033[31m===> type error!\033[0m" << std::endl;
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
		std::cout << CliSocket::GetCliSocket()->GetErrMsg() << std::endl;
		return IM_FALSE;
	}

	char RspBuffer[1024 * 1024] = { 0 };
	//recv(fd, RspBuffer, 1024 * 1024, 0);
	int len;
	res = CliSocket::GetCliSocket()->RecvBuf(RspBuffer, 1024 * 1024, len);
	if (res != SOCK_SUC)
	{
		std::cout << CliSocket::GetCliSocket()->GetErrMsg() << std::endl;
		return IM_FALSE;
	}

	IM::User::Msg::IMRspMsg Msg2;
	PDUHEAD* pRspHeader = (PDUHEAD*)RspBuffer;
	std::string RspMsg = pRspHeader->GetBody();
	Msg2.ParseFromString(RspMsg);
	int ret = Msg2.ret();
	std::string msg = Msg2.msg();
	std::cout << "\033[34m------->SERVER: " << msg << "\033[0m" << std::endl;
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

int Exit::Process()
{
	exit(0);
}

int SelectQues::Process()
{
	
}

int InsertQues::Process()
{
	return 0;
}

int GetAllQues::Process()
{
	return 0;
}

int DelQues::Process()
{
	return 0;
}
