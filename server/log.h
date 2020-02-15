#ifndef _H_LOG_H_
#define _H_LOG_H_

#include <cstdio>

#include <chrono>
#include<iostream>

#define GetDateTime() { \
	time_t nSeconds; \
	struct tm* pTM; \
	time(&nSeconds); \
	pTM = localtime(&nSeconds); \
	fprintf(stdout, "===Author: Daijia===%04d-%02d-%02d %02d:%02d:%02d == ", \
	pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday, \
	pTM->tm_hour, pTM->tm_min, pTM->tm_sec); }

//#define CONFIG_TIME_LOG 1
//#define CONFIG_ERROR_LOG 1


#if CONFIG_ERROR_LOG
#define LOGE(fmt, ...) {char log_buff[1024]; snprintf(log_buff, 1024, fmt, ##__VA_ARGS__); std::cout<<"===Author: Daijia=== ERROR: "<<log_buff<<std::endl;}
#else
#define LOGE(fmt, ...)
#endif

#define LOGI(fmt, ...) {GetDateTime(); char log_buff[1024]; snprintf(log_buff, 1024, fmt, ##__VA_ARGS__); std::cout<<log_buff<<std::endl;}

#define COMMON_DEBUG LOGI("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__)

#if CONFIG_TIME_LOG
#define LOGP(fmt, ...) {char log_buff[1024]; snprintf(log_buff, 1024, "[profile]: == " fmt, ##__VA_ARGS__); std::cout<<log_buff<<std::endl;}
#define __TIC1__(tag) auto time_##tag##_start = std::chrono::high_resolution_clock::now()
#define __TOC1__(tag) auto time_##tag##_end = std::chrono::high_resolution_clock::now();\
		std::cout<<"===Author: Daijia=== TIME: "; \
        LOGP("%s: %f ms", #tag, static_cast<float>(std::chrono::duration_cast<std::chrono::microseconds>(time_##tag##_end - time_##tag##_start).count()) / 1000.f)
#else
#define LOGP(fmt, ...)
#define __TIC1__(tag)
#define __TOC1__(tag)
#endif


#endif