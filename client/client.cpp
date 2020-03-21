#include "client.h"

Client::Client()
{
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
		std::cout << "---> please input choice: ";
		std::cin >> choice;
		if(std::cin.fail())
		{
			std::string str;
			std::cin.clear();
			std::cin >> str;
			std::cin.ignore();
			std::cout << "\033[31m===> input error!\033[0m" << std::endl;
			continue;
		}
		
		switch (choice)
		{
		case REGISTER: _control->_model[REGISTER]->Process();
			break;
		case LOGIN: DealLogin();
			break;
		case EXIT: _control->_model[EXIT]->Process();
			break;
		default:
			std::cout << "\033[31m===> input error!\033[0m" << std::endl;
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
		std::cout << "---> please input choice: ";
		std::cin >> choice;
		if (std::cin.fail())
		{
			std::string str;
			std::cin.clear();
			std::cin >> str;
			std::cin.ignore();
			std::cout << "\033[31m===> input error!\033[0m" << std::endl;
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
			std::cout << "\033[31m===> input error!\033[0m" << std::endl;
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
		std::cout << "---> please input choice: ";
		std::cin >> choice;
		if (std::cin.fail())
		{
			std::string str;
			std::cin.clear();
			std::cin >> str;
			std::cin.ignore();
			std::cout << "\033[31m===> input error!\033[0m" << std::endl;
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
		else
		{
			std::cout << "\033[31m===> input error!\033[0m" << std::endl;
			continue;
		}
	}
}


void Client::Put()
{
	GetDateTime();
	std::cout << std::endl;
	std::cout << "\033[36m-------------- Register : 1 --------------\033[0m" << std::endl;
	std::cout << "\033[36m-------------- Login  :   2 --------------\033[0m" << std::endl;
	std::cout << "\033[36m-------------- Exit   :   3 --------------\033[0m" << std::endl;
}

void Client::StudentPut()
{
	GetDateTime();
	std::cout << std::endl;
	std::cout << "\033[36m-----------------STUDENT------------------\033[0m" << std::endl;
	std::cout << "\033[36m--------------   抽题 : 1   --------------\033[0m" << std::endl;
	std::cout << "\033[36m--------------   退出 : 2   --------------\033[0m" << std::endl;
}

void Client::AdminPut()
{
	GetDateTime();
	std::cout << std::endl;
	std::cout << "\033[36m--------------------ADMIN-------------------\033[0m" << std::endl;
	std::cout << "\033[36m--------------   查看题库 : 1 --------------\033[0m" << std::endl;
	std::cout << "\033[36m--------------   插入题目 : 2 --------------\033[0m" << std::endl;
	std::cout << "\033[36m--------------   删除题目 : 3 --------------\033[0m" << std::endl;
	std::cout << "\033[36m--------------    退出 : 4    --------------\033[0m" << std::endl;
}

void Client::DealLogin()
{
	int ret = _control->_model[LOGIN]->Process();
	if (ret == IM_FALSE)
	{
		return;
	}

	if(ret == STUDENT)
	{
		StudentRun();
	}
	else if(ret == ADMIN)
	{
		TeacherRun();
	}
}
