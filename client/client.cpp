#include "client.h"

Client::Client(std::string ip, const int port)
{
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
		std::cout << "---> please input choice: ";
		std::cin >> choice;

		switch (choice)
		{
		case REGISTER: _control->_model[REGISTER]->Process();
			break;
		case LOGIN: DealLogin();
			break;
		case EXIT: _control->_model[EXIT]->Process();
			break;
		default:
			std::cout << "===> input error!" << std::endl;
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
			std::cout << "===> input error!" << std::endl;
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
			std::cout << "===> input error!" << std::endl;
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
