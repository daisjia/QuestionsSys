#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include<string>
#include<iostream>
#include <time.h>

#define _DATETIME_SIZE  32

int GetDateTime(char* psDateTime);
void LOG(std::string message);