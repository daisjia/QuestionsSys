#include "client.h"
extern timeval tv;

Client::Client()
{
	gettimeofday(&tv, NULL);
	tv.tv_sec += 60 * 60;
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(5001);
	saddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	int res = connect(fd, (struct sockaddr*) & saddr, sizeof(saddr));
	if (res == -1)
	{
		close(fd);
		LOGI("client start fail, connect == -1 !");
		exit(0);
	}
	char RspBuffer[1024] = { 0 };
	Json::Value val;
	Json::Reader reader;
	res = recv(fd, RspBuffer, 1024, 0);
	if (res <= 0)
	{
		close(fd);
		LOGI("client start fail, recv <= 0 !");
		exit(0);
	}

	if (reader.parse(RspBuffer, val) == -1)
	{
		close(fd);
		LOGI("client start fail, json parse fail!");
		exit(0);
	}

	std::string ip = val["ip"].asString();
	int port = val["port"].asInt();
	if (port < 5002)
	{
		close(fd);
		LOGI("client start fail, port < 5002");
		exit(0);
	}
	close(fd);
	CliSocket* cli = CliSocket::GetCliSocket();
	cli->Init(ip.c_str(), port, true);
	_clifd = cli->Connect();
	if (_clifd <= 0)
	{
		std::cout << cli->GetErrMsg() << std::endl;
		exit(0);
	}
	_remoteIp = ip;
	_remotePort = port;
	_control.reset(new Control);
}

std::string Client::GetRemoteIp()
{
	return _remoteIp;
}

int Client::GetRemotePort()
{
	return _remotePort;
}


void Client::Run()
{
	int choice = 0;
	while (true)
	{
		Put();
		choice = 0;
		std::cout << "---> ��ѡ��: ";
		std::cin >> choice;
		if (std::cin.fail())
		{
			std::string str;
			std::cin.clear();
			std::cin >> str;
			std::cin.ignore();
			std::cout << "\033[31m[��  ʾ]: �������\033[0m" << std::endl;
			continue;
		}

		switch (choice)
		{
		case REGISTER: _control->_model[REGISTER]->Process();
			break;
		case LOGIN: DealLogin();
			break;
		case EXIT: return;
			break;
		default:
			std::cout << "\033[31m[��  ʾ]: �������\033[0m" << std::endl;
			break;
		}
	}
}

void Client::StudentRun()
{
	int choice;
	while (true)
	{
		StudentPut();
		std::cout << "---> ��ѡ��: ";
		std::cin >> choice;
		if (std::cin.fail())
		{
			std::string str;
			std::cin.clear();
			std::cin >> str;
			std::cin.ignore();
			std::cout << "\033[31m[��  ʾ]: �������\033[0m" << std::endl;
			continue;
		}

		if (choice == 1)
		{
			_control->_model[SELECT]->Process();
		}
		else if (choice == 2)
		{
			return;
		}
		else
		{
			std::cout << "\033[31m[��  ʾ]: �������\033[0m" << std::endl;
			continue;
		}
	}
}

void Client::TeacherRun()
{
	int choice;
	while (true)
	{
		AdminPut();
		std::cout << "---> ��ѡ��: ";
		std::cin >> choice;
		if (std::cin.fail())
		{
			std::string str;
			std::cin.clear();
			std::cin >> str;
			std::cin.ignore();
			std::cout << "\033[31m[��  ʾ]: �������\033[0m" << std::endl;
			continue;
		}

		if (choice == 1)
		{
			_control->_model[GETALL]->Process();
		}
		else if (choice == 2)
		{
			_control->_model[INSERT]->Process();
		}
		else if (choice == 3)
		{
			_control->_model[DELETE]->Process();
		}
		else if (choice == 4)
		{
			_control->_model[FLUSH]->Process();
		}
		else if (choice == 5)
		{
			return;
		}
		else
		{
			std::cout << "\033[31m[��  ʾ]: �������\033[0m" << std::endl;
			continue;
		}
	}
}


void Client::Put()
{
	time_t nSeconds;
	struct tm* pTM;
	time(&nSeconds);
	pTM = localtime(&nSeconds);
	char Buffer[100] = { 0 };
	sprintf(Buffer, "*--------%04d-%02d-%02d %02d:%02d:%02d---------*",pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday, pTM->tm_hour, pTM->tm_min, pTM->tm_sec);

	std::cout << "\033[36m            ------------\033[0m" << std::endl;
	std::cout << "\033[36m            --��    ��--\033[0m" << std::endl;
	std::cout << "\033[36m            ------------\033[0m" << std::endl;
	std::cout << "\033[36m" << Buffer << "\033[0m" << std::endl;
	std::cout << "\033[36m*------------------------------------*\033[0m" << std::endl;
	std::cout << "\033[36m*----------- ע    �� : 1 -----------*\033[0m" << std::endl;
	std::cout << "\033[36m*----------- ��    ¼ : 2 -----------*\033[0m" << std::endl;
	std::cout << "\033[36m*----------- ��    �� : 3 -----------*\033[0m" << std::endl;
	std::cout << "\033[36m*------------------------------------*\033[0m" << std::endl;
}

void Client::StudentPut()
{
	time_t nSeconds;
	struct tm* pTM;
	time(&nSeconds);
	pTM = localtime(&nSeconds);
	char Buffer[100] = { 0 };
	sprintf(Buffer, "*--------%04d-%02d-%02d %02d:%02d:%02d---------*", pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday, pTM->tm_hour, pTM->tm_min, pTM->tm_sec);

	std::cout << "\033[36m            ------------\033[0m" << std::endl;
	std::cout << "\033[36m            --ѧ    ��--\033[0m" << std::endl;
	std::cout << "\033[36m            ------------\033[0m" << std::endl;
	std::cout << "\033[36m" << Buffer << "\033[0m" << std::endl;
	std::cout << "\033[36m*------------------------------------*\033[0m" << std::endl;
	std::cout << "\033[36m*----------- ��    �� : 1 -----------*\033[0m" << std::endl;
	std::cout << "\033[36m*----------- ��    �� : 2 -----------*\033[0m" << std::endl;
	std::cout << "\033[36m*------------------------------------*\033[0m" << std::endl;
}

void Client::AdminPut()
{
	time_t nSeconds;
	struct tm* pTM;
	time(&nSeconds);
	pTM = localtime(&nSeconds);
	char Buffer[100] = { 0 };
	sprintf(Buffer, "*--------%04d-%02d-%02d %02d:%02d:%02d---------*", pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday, pTM->tm_hour, pTM->tm_min, pTM->tm_sec);


	std::cout << "\033[36m            ------------\033[0m" << std::endl;
	std::cout << "\033[36m            --��    ʦ--\033[0m" << std::endl;
	std::cout << "\033[36m            ------------\033[0m" << std::endl;
	std::cout << "\033[36m" << Buffer << "\033[0m" << std::endl;
	std::cout << "\033[36m*------------------------------------*\033[0m" << std::endl;
	std::cout << "\033[36m*----------- �鿴��� : 1 -----------*\033[0m" << std::endl;
	std::cout << "\033[36m*----------- ������Ŀ : 2 -----------*\033[0m" << std::endl;
	std::cout << "\033[36m*----------- ɾ����Ŀ : 3 -----------*\033[0m" << std::endl;
	std::cout << "\033[36m*----------- ˢ����� : 4 -----------*\033[0m" << std::endl;
	std::cout << "\033[36m*----------- ��    �� : 5 -----------*\033[0m" << std::endl;
	std::cout << "\033[36m*------------------------------------*\033[0m" << std::endl;
}

void Client::DealLogin()
{
	int ret = _control->_model[LOGIN]->Process();
	if (ret == IM_FALSE)
	{
		return;
	}

	if (ret == STUDENT)
	{
		StudentRun();
	}
	else if (ret == ADMIN)
	{
		TeacherRun();
	}
}
