#pragma once
#include<iostream>
#include<functional>
#include<string>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<map>
#include"IM.User.Msg.pb.h"
#include"redis.h"
#include"mysql.h"
#include"pdu.h"
#include"log.h"
#include"ser_socket.h"

bool Register(int fd, std::string ReqMsg);
bool Login(int fd, std::string ReqMsg);
bool SelectQues(int fd, std::string ReqMsg);
bool InsertQues(int fd, std::string ReqMsg);
bool DelQues(int fd, std::string ReqMsg);
bool GetAllQues(int fd, std::string ReqMsg);

class Control
{
public:
	Control();
	~Control();
	std::map<int, std::function<bool(int, std::string)>> _model;
};

void SendResult(int fd, int cmd, int ret, std::string Msg);