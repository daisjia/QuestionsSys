#include "view.h"

std::map<int, std::string> mp1;
std::map<int, std::string> mp2;
timeval tv;

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
	std::cout << "---> ������ID: ";
	std::cin >> id;
	while (std::cin.fail())
	{
		std::string str;
		std::cin.clear();
		std::cin >> str;
		std::cin.ignore();

		std::cout << "\033[31m[��  ʾ]: ����ID����\033[0m" << std::endl;
		std::cout << "---> ���ٴ�����ID: ";
		std::cin >> id;
	}

	std::cout << "---> ����������: ";
	std::cin >> name;
	std::cout << "---> ����������: ";
	std::cin >> passwd;
	std::cout << "\033[36m*------- ����: ѧ�� 1--��ʦ 2 -------*\033[0m" << std::endl;

	std::cout << "---> ��ѡ������: ";
	std::cin >> type;
	while (std::cin.fail())
	{
		std::string str;
		std::cin.clear();
		std::cin >> str;
		std::cin.ignore();
		std::cout << "\033[31m[��  ʾ]: �������ʹ���\033[0m" << std::endl;
		std::cout << "---> ���ٴ���������: ";
		std::cin >> type;
	}

	if (type != STUDENT && type != ADMIN)
	{
		std::cout << "\033[31m[��  ʾ]: �������ʹ���\033[0m" << std::endl;
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
		std::cout << "\033[34m[��  ʾ]: ����������\033[0m" << std::endl;
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
		std::cout << "\033[34m[��  ʾ]: ����������\033[0m" << std::endl;
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
	std::cout << "\033[34m[��  ʾ]: " << msg << "\033[0m" << std::endl;
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
	std::cout << "---> ������ID: ";
	std::cin >> id;
	while (std::cin.fail())
	{
		std::string str;
		std::cin.clear();
		std::cin >> str;
		std::cin.ignore();
		std::cout << "\033[31m[��  ʾ]: ����ID����\033[0m" << std::endl;
		std::cout << "---> ���ٴ�����ID: ";
		std::cin >> id;
	}

	std::cout << "---> ����������: ";
	std::cin >> passwd;
	std::cout << "\033[36m*------- ����: ѧ�� 1--��ʦ 2 -------*\033[0m" << std::endl;

	std::cout << "---> ��ѡ������: ";
	std::cin >> type;
	while (std::cin.fail())
	{
		std::string str;
		std::cin.clear();
		std::cin >> str;
		std::cin.ignore();
		std::cout << "\033[31m[��  ʾ]: �������ʹ���\033[0m" << std::endl;
		std::cout << "---> ���ٴ���������: ";
		std::cin >> type;
	}
	if (type != STUDENT && type != ADMIN)
	{
		std::cout << "\033[31m[��  ʾ]: �������ʹ���\033[0m" << std::endl;
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

	int res = CliSocket::GetCliSocket()->SendBuf(ReqBuffer, pReqHeader.GetHeadLen() + ReqMsg.size());
	if (res != SOCK_SUC)
	{
		std::cout << "\033[34m[��  ʾ]: ����������\033[0m" << std::endl;
		sleep(1);
		exit(0);
	}

	char RspBuffer[1024 * 1024] = { 0 };
	int len;
	res = CliSocket::GetCliSocket()->RecvBuf(RspBuffer, 1024 * 1024, len);
	if (res != SOCK_SUC)
	{
		std::cout << "\033[34m[��  ʾ]: ����������\033[0m" << std::endl;
		sleep(1);
		exit(0);
	}

	IM::User::Msg::IMRspMsg Msg2;
	PDUHEAD* pRspHeader = (PDUHEAD*)RspBuffer;
	std::string RspMsg = pRspHeader->GetBody();
	Msg2.ParseFromString(RspMsg);
	int ret = Msg2.ret();
	std::string msg = Msg2.msg();
	std::cout << "\033[34m[��  ʾ]: " << msg << "\033[0m" << std::endl;
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
	srand(time(NULL));
	timeval time_out;
	gettimeofday(&time_out, NULL);
	if (mp1.size() == 0 || mp2.size() == 0 || (time_out.tv_sec > tv.tv_sec || (time_out.tv_sec == tv.tv_sec && time_out.tv_usec >= tv.tv_usec)))
	{
		gettimeofday(&tv, NULL);
		tv.tv_sec += 60 * 60;
		mp1.clear();
		mp2.clear();
		PDUHEAD pReqHeader;
		pReqHeader.SetCmd(GETALL);
		char ReqBuffer[1024] = { 0 };
		memcpy(ReqBuffer, (void*)&pReqHeader, pReqHeader.GetHeadLen());
		((PDUHEAD*)ReqBuffer)->SetPackLen(pReqHeader.GetHeadLen());
		int res = CliSocket::GetCliSocket()->SendBuf(ReqBuffer, pReqHeader.GetHeadLen());
		if (res != SOCK_SUC)
		{
			std::cout << "\033[34m[��  ʾ]: ����������\033[0m" << std::endl;
			sleep(1);
			exit(0);
		}

		char RspBuffer[1024 * 1024] = { 0 };
		int len;
		res = CliSocket::GetCliSocket()->RecvBuf(RspBuffer, 1024 * 1024, len);
		if (res != SOCK_SUC)
		{
			std::cout << "\033[34m[��  ʾ]: ����������\033[0m" << std::endl;
			sleep(1);
			exit(0);
		}

		IM::User::Msg::IMRspMsg Msg2;
		PDUHEAD* pRspHeader = (PDUHEAD*)RspBuffer;
		std::string RspMsg = pRspHeader->GetBody();
		Msg2.ParseFromString(RspMsg);
		int ret = Msg2.ret();
		std::string msg = Msg2.msg();
		if (ret == IM_OK)
		{
			int i = 1;
			int pos = msg.find('\n') + 1;
			msg = msg.substr(pos);
			while (pos != -1)
			{
				int pos1 = msg.find("[==]");
				if (pos1 == -1)
					break;
				mp1[i] = msg.substr(0, pos1);
				pos = msg.find('\n');
				int len = pos - pos1 - 4;
				mp2[i] = msg.substr(pos1 + 4, len);
				i++;
				msg = msg.substr(pos + 1);
			}
		}
		else
		{
			std::cout << "\033[34m[��  ʾ]: " << msg << "\033[0m" << std::endl;
			return IM_FALSE;
		}
	}

	int degree = 0;
	int errnum = 0;
	std::cout << "---> �������Ѷ�ϵ��[1--5]: ";
	std::cin >> degree;
	while (std::cin.fail())
	{
		errnum++;
		if (errnum >= 3)
		{
			std::cout << "\033[31m[��  ʾ]: �������������\033[0m" << std::endl;
			return IM_FALSE;
		}
		std::string str;
		std::cin.clear();
		std::cin >> str;
		std::cin.ignore();

		std::cout << "\033[31m[��  ʾ]: �������ʹ���\033[0m" << std::endl;
		std::cout << "---> ���ٴ�����: ";
		std::cin >> degree;
	}
	while (degree <= 0 || degree > 5)
	{
		errnum++;
		if (errnum >= 3)
		{
			std::cout << "\033[31m[��  ʾ]: �������������\033[0m" << std::endl;
			return IM_FALSE;
		}
		std::cout << "\033[31m[��  ʾ]: �������ʹ���\033[0m" << std::endl;
		std::cout << "---> ���ٴ�����: ";
		std::cin >> degree;
		while (std::cin.fail())
		{
			errnum++;
			if (errnum >= 3)
			{
				std::cout << "\033[31m[��  ʾ]: �������������\033[0m" << std::endl;
				return IM_FALSE;
			}
			std::string str;
			std::cin.clear();
			std::cin >> str;
			std::cin.ignore();

			std::cout << "\033[31m[��  ʾ]: �������ʹ���\033[0m" << std::endl;
			std::cout << "---> ���ٴ�����: ";
			std::cin >> degree;
		}
	}

	std::string list = "list:" + std::to_string(degree);
	std::string stack = "stack:" + std::to_string(degree);
	std::string queue = "queue:" + std::to_string(degree);
	std::string str = "string:" + std::to_string(degree);
	std::string tree = "tree:" + std::to_string(degree);

	std::vector<std::string> vc;

	for (auto it : mp1)
	{
		if (it.second.find(list.data()) == -1 && it.second.find(str.data()) == -1 && it.second.find(stack.data()) == -1 && it.second.find(queue.data()) == -1 && it.second.find(tree.data()) == -1)
		{
			continue;
		}
		vc.push_back(mp2[it.first]);
	}

	
	std::cout << "\033[34m[��  ʾ]: " << "��������: " << "\033[0m" << std::endl << std::endl;

	if (vc.size() <= 5)
	{
		int i = 1;
		for (auto it : vc)
		{
			std::cout << "\033[35m->[" << i << "]: " << it << "\033[0m" << std::endl;
			i++;
		}
		return IM_OK;
	}

	std::set<int> st;
	for (int i=0; i<5; ++i)
	{
		int j = rand() % vc.size() + 0;
		auto it = st.find(j);
		while (it != st.end())
		{
			j = rand() % vc.size() + 0;
			it = st.find(j);
		}
		st.insert(j);
		std::cout << "\033[35m->[" << i << "]: " << vc[j] << "\033[0m" << std::endl;
	}
	return IM_OK;
}

int InsertQues::Process()
{
	int errnum = 0;   //�������
	int degree = 0;
	int type = 0;
	std::string str;

	std::cout << "\033[36m*-- String: 1  List : 2  Stack : 3  Queue : 4  Tree : 5 --*\033[0m" << std::endl;
	std::cout << "---> ��ѡ������: ";
	std::cin >> type;
	while (std::cin.fail())
	{
		errnum++;
		if (errnum >= 3)
		{
			std::cout << "\033[31m[��  ʾ]: �������������\033[0m" << std::endl;
			return IM_FALSE;
		}
		std::string str;
		std::cin.clear();
		std::cin >> str;
		std::cin.ignore();

		std::cout << "\033[31m[��  ʾ]: �������ʹ���\033[0m" << std::endl;
		std::cout << "---> ���ٴ�����: ";
		std::cin >> type;
	}
	while (type <= 0 || type > 5)
	{
		errnum++;
		if (errnum >= 3)
		{
			std::cout << "\033[31m[��  ʾ]: �������������\033[0m" << std::endl;
			return IM_FALSE;
		}
		std::cout << "\033[31m[��  ʾ]: �������ʹ���\033[0m" << std::endl;
		std::cout << "---> ���ٴ�����: ";
		std::cin >> type;
		while (std::cin.fail())
		{
			errnum++;
			if (errnum >= 3)
			{
				std::cout << "\033[31m[��  ʾ]: �������������\033[0m" << std::endl;
				return IM_FALSE;
			}
			std::string str;
			std::cin.clear();
			std::cin >> str;
			std::cin.ignore();

			std::cout << "\033[31m[��  ʾ]: �������ʹ���\033[0m" << std::endl;
			std::cout << "---> ���ٴ�����: ";
			std::cin >> type;
		}
	}
	errnum = 0;
	std::cout << "---> �������Ѷ�ϵ��[1--5]: ";
	std::cin >> degree;
	while (std::cin.fail())
	{
		errnum++;
		if (errnum >= 3)
		{
			std::cout << "\033[31m[��  ʾ]: �������������\033[0m" << std::endl;
			return IM_FALSE;
		}
		std::string str;
		std::cin.clear();
		std::cin >> str;
		std::cin.ignore();

		std::cout << "\033[31m[��  ʾ]: �������ʹ���\033[0m" << std::endl;
		std::cout << "---> ���ٴ�����: ";
		std::cin >> degree;
	}
	while (degree <= 0 || degree > 5)
	{
		errnum++;
		if (errnum >= 3)
		{
			std::cout << "\033[31m[��  ʾ]: �������������\033[0m" << std::endl;
			return IM_FALSE;
		}
		std::cout << "\033[31m[��  ʾ]: �������ʹ���\033[0m" << std::endl;
		std::cout << "---> ���ٴ�����: ";
		std::cin >> degree;
		while (std::cin.fail())
		{
			errnum++;
			if (errnum >= 3)
			{
				std::cout << "\033[31m[��  ʾ]: �������������\033[0m" << std::endl;
				return IM_FALSE;
			}
			std::string str;
			std::cin.clear();
			std::cin >> str;
			std::cin.ignore();

			std::cout << "\033[31m[��  ʾ]: �������ʹ���\033[0m" << std::endl;
			std::cout << "---> ���ٴ�����: ";
			std::cin >> degree;
		}
	}

	std::cout << "---> ��������������: ";
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
	Msg.set_type(type + 30);
	Msg.set_msg(str);
	std::string ReqMsg;
	Msg.SerializeToString(&ReqMsg);
	memcpy(ReqBuffer, (void*)&pReqHeader, pReqHeader.GetHeadLen());
	memcpy(ReqBuffer + pReqHeader.GetHeadLen(), ReqMsg.c_str(), ReqMsg.size());
	((PDUHEAD*)ReqBuffer)->SetPackLen(pReqHeader.GetHeadLen() + ReqMsg.size());
	int res = CliSocket::GetCliSocket()->SendBuf(ReqBuffer, pReqHeader.GetHeadLen() + ReqMsg.size());
	if (res != SOCK_SUC)
	{
		std::cout << "\033[34m[��  ʾ]: ����������\033[0m" << std::endl;
		sleep(1);
		exit(0);
	}

	char RspBuffer[1024 * 1024] = { 0 };
	int len;
	res = CliSocket::GetCliSocket()->RecvBuf(RspBuffer, 1024 * 1024, len);
	if (res != SOCK_SUC)
	{
		std::cout << "\033[34m[��  ʾ]: ����������\033[0m" << std::endl;
		sleep(1);
		exit(0);
	}

	IM::User::Msg::IMRspMsg Msg2;
	PDUHEAD* pRspHeader = (PDUHEAD*)RspBuffer;
	std::string RspMsg = pRspHeader->GetBody();
	Msg2.ParseFromString(RspMsg);
	int ret = Msg2.ret();
	std::string msg = Msg2.msg();
	std::cout << "\033[34m[��  ʾ]: " << msg << "\033[0m" << std::endl;
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
	timeval time_out;
	gettimeofday(&time_out, NULL);
	if (mp1.size() == 0 || mp2.size() == 0 || (time_out.tv_sec > tv.tv_sec || (time_out.tv_sec == tv.tv_sec && time_out.tv_usec >= tv.tv_usec)))
	{
		gettimeofday(&tv, NULL);
		tv.tv_sec += 60 * 60;
		mp1.clear();
		mp2.clear();
		PDUHEAD pReqHeader;
		pReqHeader.SetCmd(GETALL);
		char ReqBuffer[1024] = { 0 };
		memcpy(ReqBuffer, (void*)&pReqHeader, pReqHeader.GetHeadLen());
		((PDUHEAD*)ReqBuffer)->SetPackLen(pReqHeader.GetHeadLen());
		int res = CliSocket::GetCliSocket()->SendBuf(ReqBuffer, pReqHeader.GetHeadLen());
		if (res != SOCK_SUC)
		{
			std::cout << "\033[34m[��  ʾ]: ����������\033[0m" << std::endl;
			sleep(1);
			exit(0);
		}

		char RspBuffer[1024 * 1024] = { 0 };
		int len;
		res = CliSocket::GetCliSocket()->RecvBuf(RspBuffer, 1024 * 1024, len);
		if (res != SOCK_SUC)
		{
			std::cout << "\033[34m[��  ʾ]: ����������\033[0m" << std::endl;
			sleep(1);
			exit(0);
		}

		IM::User::Msg::IMRspMsg Msg2;
		PDUHEAD* pRspHeader = (PDUHEAD*)RspBuffer;
		std::string RspMsg = pRspHeader->GetBody();
		Msg2.ParseFromString(RspMsg);
		int ret = Msg2.ret();
		std::string msg = Msg2.msg();
		/*
		list:4*id:100[==]��ת������
		list:4*id:6104[==]���õ�����
		stack:2*id:9502[==]ջʵ�ֶ���
		*/
		if (ret == IM_OK)
		{
			int i = 1;
			int pos = msg.find('\n') + 1;
			msg = msg.substr(pos);
			while (pos != -1)
			{
				int pos1 = msg.find("[==]");
				if (pos1 == -1)
					break;
				mp1[i] = msg.substr(0, pos1);
				pos = msg.find('\n');
				int len = pos - pos1 - 4;
				mp2[i] = msg.substr(pos1 + 4, len);
				i++;
				msg = msg.substr(pos + 1);
			}
		}
		else
		{
			std::cout << "\033[34m[��  ʾ]: " << msg << "\033[0m" << std::endl;
			return IM_FALSE;
		}
	}
	std::cout << "\033[34m[��  ʾ]: " << "������������: " << "\033[0m" << std::endl << std::endl;

	for (auto it : mp2)
	{
		std::cout << "\033[35m->[" << it.first << "]: " << it.second << "\033[0m" << std::endl;
	}
}

int DelQues::Process()
{
	if (mp1.size() == 0 || mp2.size() == 0)
	{
		FlushQues f;
		f.Process();
	}
	std::cout << "\033[34m[��  ʾ]: " << "������������: " << "\033[0m" << std::endl << std::endl;
	for (auto it : mp2)
	{
		std::cout << "\033[35m->[" << it.first << "]: " << it.second << "\033[0m"<<std::endl;
	}

	int num = 0;
	int errnum = 0;
	int size = mp1.size();
	std::cout << "---> ��������Ҫɾ�������: ";
	std::cin >> num;
	while (std::cin.fail())
	{
		errnum++;
		if (errnum >= 3)
		{
			std::cout << "\033[31m[��  ʾ]: �������������\033[0m" << std::endl;
			return IM_FALSE;
		}
		std::string str;
		std::cin.clear();
		std::cin >> str;
		std::cin.ignore();

		std::cout << "\033[31m[��  ʾ]: �������ʹ���\033[0m" << std::endl;
		std::cout << "---> ���ٴ�����: ";
		std::cin >> num;
	}
	while (num <= 0 || num > size)
	{
		errnum++;
		if (errnum >= 3)
		{
			std::cout << "\033[31m[��  ʾ]: �������������\033[0m" << std::endl;
			return IM_FALSE;
		}
		std::cout << "\033[31m[��  ʾ]: �������ʹ���\033[0m" << std::endl;
		std::cout << "---> ���ٴ�����: ";
		std::cin >> num;
		while (std::cin.fail())
		{
			errnum++;
			if (errnum >= 3)
			{
				std::cout << "\033[31m[��  ʾ]: �������������\033[0m" << std::endl;
				return IM_FALSE;
			}
			std::string str;
			std::cin.clear();
			std::cin >> str;
			std::cin.ignore();

			std::cout << "\033[31m[��  ʾ]: �������ʹ���\033[0m" << std::endl;
			std::cout << "---> ���ٴ�����: ";
			std::cin >> num;
		}
	}

	auto it = mp1.find(num);
	if (it == mp1.end())
	{
		std::cout << "\033[31m[��  ʾ]: ɾ��ʧ��\033[0m" << std::endl;
		return IM_FALSE;
	}

	PDUHEAD pReqHeader;
	pReqHeader.SetCmd(DELETE);
	char ReqBuffer[1024 * 1024] = { 0 };
	IM::User::Msg::IMRspMsg Msg;
	Msg.set_ret(num);
	Msg.set_msg(it->second.data());
	std::string ReqMsg;
	Msg.SerializeToString(&ReqMsg);
	memcpy(ReqBuffer, (void*)&pReqHeader, pReqHeader.GetHeadLen());
	memcpy(ReqBuffer + pReqHeader.GetHeadLen(), ReqMsg.c_str(), ReqMsg.size());
	((PDUHEAD*)ReqBuffer)->SetPackLen(pReqHeader.GetHeadLen() + ReqMsg.size());
	int res = CliSocket::GetCliSocket()->SendBuf(ReqBuffer, pReqHeader.GetHeadLen() + ReqMsg.size());
	if (res != SOCK_SUC)
	{
		std::cout << "\033[34m[��  ʾ]: ����������\033[0m" << std::endl;
		sleep(1);
		exit(0);
	}

	char RspBuffer[1024 * 1024] = { 0 };
	int len;
	res = CliSocket::GetCliSocket()->RecvBuf(RspBuffer, 1024 * 1024, len);
	if (res != SOCK_SUC)
	{
		std::cout << "\033[34m[��  ʾ]: ����������\033[0m" << std::endl;
		sleep(1);
		exit(0);
	}

	IM::User::Msg::IMRspMsg Msg2;
	PDUHEAD* pRspHeader = (PDUHEAD*)RspBuffer;
	std::string RspMsg = pRspHeader->GetBody();
	Msg2.ParseFromString(RspMsg);
	int ret = Msg2.ret();
	std::string msg = Msg2.msg();
	std::cout << "\033[34m[��  ʾ]: " << msg << "\033[0m" << std::endl;
	if (ret == IM_OK)
	{
		return IM_TRUE;
	}
	else
	{
		return IM_FALSE;
	}
}

int FlushQues::Process()
{
	gettimeofday(&tv, NULL);
	tv.tv_sec += 60 * 60;
	mp1.clear();
	mp2.clear();
	PDUHEAD pReqHeader;
	pReqHeader.SetCmd(GETALL);
	char ReqBuffer[1024] = { 0 };
	memcpy(ReqBuffer, (void*)&pReqHeader, pReqHeader.GetHeadLen());
	((PDUHEAD*)ReqBuffer)->SetPackLen(pReqHeader.GetHeadLen());
	int res = CliSocket::GetCliSocket()->SendBuf(ReqBuffer, pReqHeader.GetHeadLen());
	if (res != SOCK_SUC)
	{
		std::cout << "\033[34m[��  ʾ]: ����������\033[0m" << std::endl;
		sleep(1);
		exit(0);
	}

	char RspBuffer[1024 * 1024] = { 0 };
	int len;
	res = CliSocket::GetCliSocket()->RecvBuf(RspBuffer, 1024 * 1024, len);
	if (res != SOCK_SUC)
	{
		std::cout << "\033[34m[��  ʾ]: ����������\033[0m" << std::endl;
		sleep(1);
		exit(0);
	}

	IM::User::Msg::IMRspMsg Msg2;
	PDUHEAD* pRspHeader = (PDUHEAD*)RspBuffer;
	std::string RspMsg = pRspHeader->GetBody();
	Msg2.ParseFromString(RspMsg);
	int ret = Msg2.ret();
	std::string msg = Msg2.msg();

	if (ret == IM_OK)
	{
		int i = 1;
		int pos = msg.find('\n') + 1;
		msg = msg.substr(pos);
		while (pos != -1)
		{
			int pos1 = msg.find("[==]");
			if (pos1 == -1)
				break;
			mp1[i] = msg.substr(0, pos1);
			pos = msg.find('\n');
			int len = pos - pos1 - 4;
			mp2[i] = msg.substr(pos1 + 4, len);
			i++;
			msg = msg.substr(pos + 1);
		}
		std::cout << "\033[34m[��  ʾ]: " << "ˢ������ɹ�" << "\033[0m" << std::endl;
	}
	else
	{
		std::cout << "\033[34m[��  ʾ]: " << msg << "\033[0m" << std::endl;
		return IM_FALSE;
	}
}
