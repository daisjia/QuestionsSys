#include "control.h"


Control::Control()
{
	_model.insert(std::make_pair(REGISTER, new Register()));
	_model.insert(std::make_pair(LOGIN, new Login()));
	_model.insert(std::make_pair(EXIT, new Exit()));
}

Control::~Control()
{
	for (auto it : _model)
	{
		delete (it.second);
	}
	_model.clear();
}
