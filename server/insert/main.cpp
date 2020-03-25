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
	str.push_back(make_pair(4, "字符串匹配实现"));
	str.push_back(make_pair(5, "kmp算法"));
	str.push_back(make_pair(2, "替换空格"));
	str.push_back(make_pair(4, "正则表达式匹配"));
	str.push_back(make_pair(1, "表示数值的字符串"));
	str.push_back(make_pair(3, "字符流中第一个不重复的字符"));
	str.push_back(make_pair(4, "字符串的排列"));
	str.push_back(make_pair(5, "把数组排成最小的树"));
	vc.push_back(make_pair(1, str));

	//list
	list.push_back(make_pair(4, "判断单链表是否是有环，并找出环的入口"));
	list.push_back(make_pair(3, "链表中倒数第k个结点"));
	list.push_back(make_pair(4, "判断两个单链表是否相交，并找出交点"));
	list.push_back(make_pair(3, "逆置单链表"));
	list.push_back(make_pair(2, "从头到尾打印单链表"));
	list.push_back(make_pair(4, "删除链表中重复的结点"));
	list.push_back(make_pair(3, "合并两个排序的链表"));
	list.push_back(make_pair(5, "二叉搜索树与双向链表"));
	vc.push_back(make_pair(2, list));

	//stack
	stack.push_back(make_pair(3, "两个栈实现一个队列"));
	stack.push_back(make_pair(4, "包含min函数的栈"));
	stack.push_back(make_pair(5, "栈的牙入、弹出序列"));
	vc.push_back(make_pair(3, stack));

	//queue
	queue.push_back(make_pair(3, "两个队列实现一个栈"));
	vc.push_back(make_pair(4, queue));

	//tree
	tree.push_back(make_pair(5, "判断是否是BST树"));
	tree.push_back(make_pair(1, "中序遍历二叉树"));
	tree.push_back(make_pair(2, "层次遍历二叉树"));
	tree.push_back(make_pair(4, "树的子结构"));
	tree.push_back(make_pair(3, "按之之形打印二叉树"));
	tree.push_back(make_pair(5, "红黑树和AVL树的比较"));
	tree.push_back(make_pair(4, "重建二叉树"));
	tree.push_back(make_pair(5, "二叉树的下一个结点"));
	tree.push_back(make_pair(2, "对称的二叉树"));
	tree.push_back(make_pair(4, "序列化二叉树"));
	tree.push_back(make_pair(3, "二叉搜索树的第k个结点"));
	tree.push_back(make_pair(5, "从上往下打印二叉树"));
	tree.push_back(make_pair(1, "二叉搜索树的后序遍历序列"));
	tree.push_back(make_pair(3, "二叉树中和为某一值的路径"));
	tree.push_back(make_pair(4, "二叉树的深度"));
	tree.push_back(make_pair(5, "二叉树的元素个数"));
	tree.push_back(make_pair(2, "平衡二叉树"));
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