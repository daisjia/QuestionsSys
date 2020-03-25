#include"sys.h"
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

int main(int argc, char *argv[])
{
	srand(time(NULL));
	Sys* mysys = Sys::GetSys();
	mysys->Run();
	return 0;
}