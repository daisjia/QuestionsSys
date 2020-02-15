#pragma once
#include<iostream>
#include<functional>
#include<string>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<map>
#include"human.pb.h"
#include"redis.h"
#include"mysql.h"

enum TYPE
{
	REGISTER,
	LOGIN,
};

bool Register(int fd, std::string message);
bool Login(int fd, std::string message);

class Control
{
public:
	Control();
	~Control();
	std::map<int, std::function<bool(int, std::string)>> _model;
};

void SendResult(int fd, int ret, std::string message);