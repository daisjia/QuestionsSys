#include"sys.h"

int main()
{
	int port;
	std::cout << "-->please input port: ";
	std::cin >> port;
	Sys* mysys = Sys::GetSys("127.0.0.1", port);
	mysys->Run();
	return 0;
}