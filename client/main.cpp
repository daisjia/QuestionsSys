#include<stdlib.h>
#include<stddef.h>
#include<string>
#include<hiredis/hiredis.h>
#include<iostream>
#include"client.h"

int main()
{
	int port;
	std::cout << "please input port: ";
	std::cin >> port;
	std::unique_ptr<Client> cli;
	cli.reset(new Client("127.0.0.1", port));
	cli->Run();
	return 0;
}
