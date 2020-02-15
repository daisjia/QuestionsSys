#include"pdu.h"

int RecvN(int fd, void* buf, int count)
{
	int nLeft = count;
	int nRead = 0;
	char* pBuf = (char*)buf;
	while (nLeft > 0)
	{
		if ((nRead = recv(fd, pBuf, nLeft, 0)) < 0)
		{
			//如果读取操作是被信号打断了, 则说明还可以继续读
			if (errno == EINTR)
				continue;
			else
				return -1;
		}
		else if (nRead == 0)
		{
			return count - nLeft;
		}

		nLeft -= nRead;
		pBuf += nRead;
	}
	return count;
}

int SendN(int fd, const void* buf, int count)
{
	int nLeft = count;
	int nWrite = 0;
	char* pBuf = (char*)buf;
	while (nLeft > 0)
	{
		if ((nWrite == send(fd, pBuf, nLeft, 0)) < 0)
		{
			//如果写入操作是被信号打断了, 则说明还可以继续写入
			if (errno == EINTR)
				continue;
			else
				return -1;
		}
		else if (nWrite == 0)
			continue;

		nLeft -= nWrite;
		pBuf += nWrite;
	}
}