#pragma once
#include<iostream>
#include<sys/socket.h>
#include <netinet/in.h>


#define REGISTER 1
#define LOGIN 2
#define EXIT 3
#define GETALL 4	//获得所有试题
#define SELECT 5	//抽题
#define INSERT 6	//插入
#define DELETE 7	//删除

//试题类型
#define STRING 31
#define LIST 32
#define STACK 33
#define QUEUE 34
#define TREE 35

#define IM_OK 1
#define IM_ERROR -1

#define STUDENT 1
#define ADMIN 2

#define IM_TRUE 1
#define IM_FALSE 0


typedef struct PDUHEADtag
{
	unsigned _pack_len;	//整个包长度的低位 整个包长度==_pack_len + 65536 * _detail_info._pack_len_high;
	int _req_cmd;		//请求的命令字
	int _rsp_code;	//返回码
	unsigned short _pack_len_high;	//包长高位 整个包长度==_pack_len + 65536 * _detail_info._pack_len_high;

	PDUHEADtag()
	{
		Clear();
	}

	void Clear()
	{
		_pack_len = 0;
		_req_cmd = 0;
		_rsp_code = 0;
		_pack_len_high = 0;
	}

	void SetPackLen(unsigned int pack_len)
	{
		_pack_len = htons(pack_len % 65536);
		_pack_len_high = htons(pack_len / 65536);
	}

	unsigned int GetPackLen()
	{
		return ntohs(_pack_len) + 65536 * ntohs(_pack_len_high);
	}

	int GetHeadLen()
	{
		return sizeof(PDUHEADtag);
	}

	char* GetBody()
	{
		return ((char*)this + GetHeadLen());
	}

	void SetCmd(short cmd)
	{
		_req_cmd = htons(cmd);
	}

	int GetCmd()
	{
		return ntohs(_req_cmd);
	}
}PDUHEAD;

int RecvN(int fd, void* buf, int count);

int SendN(int fd, const void* buf, int count);