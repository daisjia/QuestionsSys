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
		case LOGIN: if (_control->_model[LOGIN]->Process()) { UserRun(); }
			break;
		case EXIT: _control->_model[EXIT]->Process();
			break;
		default:
			std::cout << "===> input error!" << std::endl;
			break;
		}
	}
}

void Client::UserRun()
{
	int choice = 0;
	while (true)
	{
		UserPut();
		std::cout << "---> please input choice: ";
		std::cin >> choice;
		auto it = _control->_model.find(choice);
		if (it == _control->_model.end())
		{
			std::cout << "===> input error!" << std::endl;
			continue;
		}
		it->second->Process();
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

void Client::UserPut()
{
	GetDateTime();
	std::cout << "------------------UserPut---------------------" << std::endl;
	std::cout << std::endl;
	std::cout << "-------------- Register : 1 --------------" << std::endl;
	std::cout << "-------------- Login  :   2 --------------" << std::endl;
	std::cout << "-------------- Exit   :   3 --------------" << std::endl;
}
