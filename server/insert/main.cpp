#include<iostream>
using namespace std;
#include<string>
#include<map>
#include"redis.h"
#include"pdu.h"

int main()
{
	srand(time(NULL));
	RedisPool* redis = RedisPool::GetRedisPool();
	redis->SetConf(2);
	redis->Exist("flushdb");
	//type       degree  shiti
	vector<pair<int, vector<pair<int, string>>>> vc;
	vector<pair<int, string>> str, list, stack, queue, tree;

	//string
	str.push_back(make_pair(4, "�ַ���ƥ��ʵ��"));
	str.push_back(make_pair(5, "kmp�㷨"));
	str.push_back(make_pair(2, "�滻�ո�"));
	str.push_back(make_pair(4, "������ʽƥ��"));
	str.push_back(make_pair(1, "��ʾ��ֵ���ַ���"));
	str.push_back(make_pair(3, "�ַ����е�һ�����ظ����ַ�"));
	str.push_back(make_pair(4, "�ַ���������"));
	str.push_back(make_pair(5, "�������ų���С����"));
	vc.push_back(make_pair(1, str));

	//list
	list.push_back(make_pair(4, "�жϵ������Ƿ����л������ҳ��������"));
	list.push_back(make_pair(3, "�����е�����k�����"));
	list.push_back(make_pair(4, "�ж������������Ƿ��ཻ�����ҳ�����"));
	list.push_back(make_pair(3, "���õ�����"));
	list.push_back(make_pair(2, "��ͷ��β��ӡ������"));
	list.push_back(make_pair(4, "ɾ���������ظ��Ľ��"));
	list.push_back(make_pair(3, "�ϲ��������������"));
	list.push_back(make_pair(5, "������������˫������"));
	vc.push_back(make_pair(2, list));

	//stack
	stack.push_back(make_pair(3, "����ջʵ��һ������"));
	stack.push_back(make_pair(4, "����min������ջ"));
	stack.push_back(make_pair(5, "ջ�����롢��������"));
	vc.push_back(make_pair(3, stack));

	//queue
	queue.push_back(make_pair(3, "��������ʵ��һ��ջ"));
	vc.push_back(make_pair(4, queue));

	//tree
	tree.push_back(make_pair(5, "�ж��Ƿ���BST��"));
	tree.push_back(make_pair(1, "�������������"));
	tree.push_back(make_pair(2, "��α���������"));
	tree.push_back(make_pair(4, "�����ӽṹ"));
	tree.push_back(make_pair(3, "��֮֮�δ�ӡ������"));
	tree.push_back(make_pair(5, "�������AVL���ıȽ�"));
	tree.push_back(make_pair(4, "�ؽ�������"));
	tree.push_back(make_pair(5, "����������һ�����"));
	tree.push_back(make_pair(2, "�ԳƵĶ�����"));
	tree.push_back(make_pair(4, "���л�������"));
	tree.push_back(make_pair(3, "�����������ĵ�k�����"));
	tree.push_back(make_pair(5, "�������´�ӡ������"));
	tree.push_back(make_pair(1, "�����������ĺ����������"));
	tree.push_back(make_pair(3, "�������к�Ϊĳһֵ��·��"));
	tree.push_back(make_pair(4, "�����������"));
	tree.push_back(make_pair(5, "��������Ԫ�ظ���"));
	tree.push_back(make_pair(2, "ƽ�������"));
	vc.push_back(make_pair(5, tree));

	for (auto it : vc)
	{
		for (auto it1 : it.second)
		{
			int type = it.first + 30;
			int degree = it1.first;
			std::string msg = it1.second;
			std::string key;
			if (type == LIST)
				key = "list:" + std::to_string(degree);
			else if (type == STRING)
				key = "string:" + std::to_string(degree);
			else if (type == STACK)
				key = "stack:" + std::to_string(degree);
			else if (type == QUEUE)
				key = "queue:" + std::to_string(degree);
			else if (type == TREE)
				key = "tree:" + std::to_string(degree);

			std::string field = key + "*id:" + std::to_string(rand() % 9999 + 1);
			std::string cmd = "exists " + key;
			int flag = RedisPool::GetRedisPool()->Exist(cmd.data());
			if (flag == -1)
			{
				cout << "redis command fail!" << endl;
				return 0;
			}
			else if (flag == 1)
			{
				cmd.clear();
				cmd = "hexists " + key + " " + field;
				flag = RedisPool::GetRedisPool()->Exist(cmd.data());
				while (flag == 1)
				{
					field.clear();
					field = key + "*id:" + std::to_string(rand() % 9999 + 1);
					flag = RedisPool::GetRedisPool()->Exist(cmd.data());
				}
				if (flag == -1)
				{
					cout << "redis command fail!" << endl;
					return 0;
				}
			}

			std::string redisCmd;
			redisCmd = "HSET " + key + " " + field + " " + msg;
			if (!RedisPool::GetRedisPool()->Insert(redisCmd.data()))
			{
				LOGI("redis command fail!");
				return 0;
			}
		}
	}

	return 0;
}