#pragma once
#include<stdlib.h>
#include<stddef.h>
#include<string>
#include<hiredis/hiredis.h>
#include<iostream>
#include"IM.User.Msg.pb.h"
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<string.h>
#include<memory>
#include<map>
#include"pdu.h"
#include"log.h"
#include"cli_socket.h"

class View
{
public:
	View() {}
	virtual ~View() {};
	virtual int Process() = 0;
};

class Register : public View
{
public:
	Register() {};
	int Process();
};

class Login : public View
{
public:
	Login() {};
	int Process();
};

class SelectQues : public View
{
public:
	SelectQues() {};
	int Process();
};

class InsertQues : public View
{
public:
	InsertQues() {};
	int Process();
};

class DelQues : public View
{
public:
	DelQues() {};
	int Process();
};

class GetAllQues : public View
{
public:
	GetAllQues() {};
	int Process();
};

class FlushQues : public View
{
public:
	FlushQues() {};
	int Process();
};
