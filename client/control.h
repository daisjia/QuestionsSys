#pragma once
#include"view.h"
#include<iostream>
#include<map>

class Control
{
public:
	Control();
	~Control();

	std::map<int, View*> _model;
};