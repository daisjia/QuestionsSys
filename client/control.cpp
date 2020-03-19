#include "control.h"


Control::Control()
{
	_model.insert(std::make_pair(REGISTER, new Register()));
	_model.insert(std::make_pair(LOGIN, new Login()));
	_model.insert(std::make_pair(EXIT, new Exit()));
	_model.insert(std::make_pair(INSERT, new InsertQues()));
	_model.insert(std::make_pair(SELECT, new SelectQues()));
	_model.insert(std::make_pair(GETALL, new GetAllQues()));
	_model.insert(std::make_pair(DELETE, new DelQues()));
}

Control::~Control()
{
	for (auto it : _model)
	{
		delete (it.second);
	}
	_model.clear();
}
