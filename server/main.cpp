#include"sys.h"
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

/*
#define CONFLINE_MAX 1024
std::map<std::string, std::string> conf;
int logfd;

std::vector<std::string> Split(std::string srcStr, const std::string& delim)
{
	int pos = 0;
	std::vector<std::string> vec;
	pos = srcStr.find(delim.c_str());
	while (-1 != pos)
	{
		std::string temp = srcStr.substr(0, pos);
		if (pos != 0)
			vec.push_back(temp);
		srcStr = srcStr.substr(pos + 1);
		pos = srcStr.find(delim.c_str());
	}
	//vec.push_back(srcStr);
	return vec;
}

void loadServerConfFromString(std::string& config)
{
	std::string blanks("\f\v\r\t\n ");
	std::vector<std::string> vec = Split(config, "\n");
	for (auto it : vec)
	{
		it.erase(0, it.find_first_not_of(blanks));
		it.erase(it.find_last_not_of(blanks) + 1);
		if (it[0] == '#' || it.size() == 0)
			continue;
		int pos = it.find(' ');
		conf[it.substr(0, pos)] = it.substr(pos + 1);
	}
}

bool loadServerConf(char* config)
{
	char buff[CONFLINE_MAX];
	std::string str;
	if (config == nullptr)
	{
		std::cout << "conf file error!" << std::endl;
		return false;
	}

	int fd = open(config, O_RDONLY);
	if (fd <= 0)
	{
		std::cout << "open conf file fail!" << std::endl;
		return false;
	}

	while (read(fd, buff, CONFLINE_MAX) != 0)
	{
		str.append(buff);
	}

	if (fd > 0)
		close(fd);

	loadServerConfFromString(str);
}
*/

int main(int argc, char *argv[])
{
	/*
	if (argc < 2)
	{
		std::cout << "not find conf file!" << std::endl;
		return 0;
	}

	loadServerConf(argv[1]);
	for (auto it : conf)
	{
		std::cout << it.first << "  " << it.second <<std::endl;
	}
	std::cout << conf.size() << std::endl;

	//std::cout << atoi(conf["port"].c_str()) << std::endl;
	
	logfd = open(conf["logfile"].c_str(), O_WRONLY);

	*/
	int port = 0;
	std::cout << "-->please input port: ";
	std::cin >> port;
	Sys* mysys = Sys::GetSys("127.0.0.1", port);
	mysys->Run();
	return 0;
}