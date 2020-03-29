#include<iostream>
using namespace std;

#include<vector>
#include<csignal>

class A
{
public:
	A() {}
	~A()
	{
		for (int i = 0; i < vc.size(); ++i)
		{
			cout << "i == " << vc[i] << endl;
		}
	}
	vector<int> vc;

};



int main()
{
	A a;
	int i = 0;
	while (1)
	{
		a.vc.push_back(i++);
	}
	return 0;
}