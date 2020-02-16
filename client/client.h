#pragma once
#include"control.h"

class Client
{
public:
	Client(std::string ip, const int port);
	std::string GetRemoteIp();
	int GetRemotePort();
	void Run();
	void UserRun();

private:
	std::string _remoteIp;
	int _remotePort;
	int _clifd;
	std::unique_ptr<Control> _control;
	void Put();
	void UserPut();
};
