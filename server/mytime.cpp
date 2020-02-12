#include "mytime.h"

int GetDateTime(char* psDateTime)
{
	time_t nSeconds;
	struct tm* pTM;
	time(&nSeconds);
	pTM = localtime(&nSeconds);
	sprintf(psDateTime, "%04d-%02d-%02d %02d:%02d:%02d",
		pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday,
		pTM->tm_hour, pTM->tm_min, pTM->tm_sec);
	return 0;
}

void LOG(std::string message)
{
	char DateTime[_DATETIME_SIZE];
	GetDateTime(DateTime);
	std::cout << "===Author: Daijia===" << " " << DateTime <<" "<< message.c_str() << std::endl;
}
