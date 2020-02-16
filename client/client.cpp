#include "client.h"

Client::Client(std::string ip, const int port)
{
	_clifd = socket(AF_INET, SOCK_STREAM, 0);
	if (_clifd == -1)
	{
		LOGE("socket create fail!");
		return;
	}

	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	saddr.sin_addr.s_addr = inet_addr(ip.c_str());
	int ret = connect(_clifd, (struct sockaddr*) & saddr, sizeof(saddr));
	if (ret == -1)
	{
		LOGE("connect server fail!");
		return;
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
		case REGISTER: _control->_model[REGISTER]->Process(_clifd); 
			break;
		case LOGIN: if (_control->_model[LOGIN]->Process(_clifd)) { UserRun(); }
			break;
		case EXIT: _control->_model[EXIT]->Process(_clifd);
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
		it->second->Process(_clifd);
	}
}

void Client::Put()
{
	GetDateTime();
	std::cout << std::endl;
	std::cout << "-------------- Register : 1 --------------" << std::endl;
	std::cout << "-------------- Login  :   2 --------------" << std::endl;
	std::cout << "-------------- Exit   :   3 --------------" << std::endl;
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
