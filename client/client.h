#pragma once
#include"control.h"
#include"cli_socket.h"
#include<json/json.h>

class Client
{
public:
	Client();
	std::string GetRemoteIp();
	int GetRemotePort();
	void Run();
	void StudentRun();
	void TeacherRun();
	void DealLogin();
private:
	std::string _remoteIp;
	int _remotePort;
	int _clifd;
	std::unique_ptr<Control> _control;
	void Put();
	void StudentPut();
	void AdminPut();
};


