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
			//�����ȡ�����Ǳ��źŴ����, ��˵�������Լ�����
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
			//���д������Ǳ��źŴ����, ��˵�������Լ���д��
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