#include<stdlib.h>
#include<stddef.h>
#include<string>
#include<hiredis/hiredis.h>
#include<iostream>
#include"client.h"

int main()
{
	std::unique_ptr<Client> cli;
	cli.reset(new Client());
	cli->Run();
	return 0;
}
