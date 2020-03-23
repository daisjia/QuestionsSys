#include "ser_socket.h"

static PDUHEAD h;
static int PDU_HEAD_SIZE = h.GetHeadLen();
SerSocket* SerSocket::ser = NULL;
std::mutex SerSocket::lck;

SerSocket::SerSocket()
{
	_sockfd = -1;
	_func = NULL;
	_hostIp = "";
	_hostPort = 0;
	SetPduProtocol();
}

SerSocket::~SerSocket()
{
	Close();
	_func = NULL;
}

int SerSocket::Init(const char* ip, int port)
{
	_hostIp = ip;
	_hostPort = port;
	bzero(&_hostAddr, sizeof(_hostAddr));
	_hostAddr.sin_family = AF_INET;
	inet_aton(_hostIp.c_str(), &_hostAddr.sin_addr);
	_hostAddr.sin_port = htons(_hostPort);
	CHECK_HOSP();
	_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (_sockfd <= 0)
	{
		snprintf(_errStr, 512, "[%s][%d]create socket ip:%s port:%d err:%s\n",
			__FILE__, __LINE__,
			_hostIp.c_str(), _hostPort, strerror(errno));
		Close();
		exit(0);
	}
	int ret = bind(_sockfd, (struct sockaddr*) & _hostAddr, sizeof(sockaddr));
	if (ret == -1)
	{
		snprintf(_errStr, 512, "[%s][%d]create socket ip:%s port:%d err:%s\n",
			__FILE__, __LINE__,
			_hostIp.c_str(), _hostPort, strerror(errno));
		Close();
		exit(0);
	}
	ret = listen(_sockfd, 20);
	if (ret == -1)
	{
		snprintf(_errStr, 512, "[%s][%d]create socket ip:%s port:%d err:%s\n",
			__FILE__, __LINE__,
			_hostIp.c_str(), _hostPort, strerror(errno));
		exit(0);
	}

	if (SetNonBlock(_sockfd) != SOCK_SUC)
	{
		snprintf(_errStr, 512, "[%s][%d]fcntl set ip:%s port:%d non-block error\n",
			__FILE__, __LINE__,
			_hostIp.c_str(), _hostPort);
		Close();
		exit(0);
	}
	return SOCK_SUC;
}

int SerSocket::Close()
{
	if (_sockfd > 0)
	{
		close(_sockfd);
		_sockfd = -1;
	}
	return SOCK_SUC;
}

void SerSocket::GetHost(std::string& ip, int& port)
{
	ip = _hostIp;
	port = _hostPort;
}

int SerSocket::Accept(sockaddr_in& caddr)
{
	CHECK_HOSP();

	socklen_t len = sizeof(caddr);
	int fd = accept(_sockfd, (struct sockaddr*) & caddr, &len);
	if (fd <= 0)
	{
		//客户端在accept之前关闭连接，作为忽略
		if (errno == EWOULDBLOCK || errno == ECONNABORTED || errno == EPROTO || errno == EINTR)
		{
			return 0;
		}
	}
	return fd;
}

int SerSocket::SendBuf(int fd, char* message, int len)
{
	CHECK_ALL();
	_errStr[0] = '\0';

	int left = len;
	int nsend = 0;
	int loop = 1000;
	for (int i = 0; i < loop; ++i)
	{
	/*	if (CheckWrite(_sockfd) <= 0)
		{
			snprintf(_errStr, 512, "[%s][%d]recv ip:%s port:%d loop:%d buf-len:%d error\n",
				__FILE__, __LINE__,
				_hostIp.c_str(), _hostPort, loop, len);
			Close();
			return ERROR_SEND;
		}*/

		int ret = send(fd, message + nsend, left, 0);
		if (ret < 0)
		{
			if (errno == EINTR || errno == EAGAIN)
			{
				continue;
			}
			snprintf(_errStr, 512, "[%s][%d]recv ip:%s port:%d loop:%d buf-len:%d error\n",
				__FILE__, __LINE__,
				_hostIp.c_str(), _hostPort, loop, len);
			return ERROR_SEND;
		}
		left -= ret;
		nsend += ret;
		if (left == 0)
			break;
	}
	if (left > 0)
	{
		snprintf(_errStr, 512, "[%s][%d]recv ip:%s port:%d loop:%d buf-len:%d error\n",
			__FILE__, __LINE__,
			_hostIp.c_str(), _hostPort, loop, len);
		return ERROR_SEND;
	}
	return SOCK_SUC;
}

int SerSocket::RecvBuf(int fd, char* buff, int needlen)
{
	//char buff[1024 * 1024]{ 0 };
	CHECK_ALL();
	_errStr[0] = '\0';
	int len = 0;
	int loop = 1000;
	for (int i = 0; i < loop; ++i)
	{
		if (CheckRead(fd) <= 0)
		{
			snprintf(_errStr, 512, "[%s][%d]recv ip:%s port:%d loop:%d buf-len:%d error\n",
				__FILE__, __LINE__,
				_hostIp.c_str(), _hostPort, loop, len);
			return ERROR_RECV;
		}

		if ((needlen - len) <= 0)
		{
			snprintf(_errStr, 512, "[%s][%d]recv ip:%s port:%d loop:%d buf-len:%d error\n",
				__FILE__, __LINE__,
				_hostIp.c_str(), _hostPort, loop, len);
			return ERROR_RECV;
		}


		int ret = recv(fd, buff + len, (needlen - len), 0);
		if (ret < 0)
		{
			if (errno == EINTR || errno == EAGAIN)
			{
				continue;
			}
			snprintf(_errStr, 512, "[%s][%d]recv ip:%s port:%d loop:%d buf-len:%d error\n",
				__FILE__, __LINE__,
				_hostIp.c_str(), _hostPort, loop, len);
			return ERROR_RECV;
		}
		if (ret == 0)
		{
			snprintf(_errStr, 512, "[%s][%d]recv ip:%s port:%d loop:%d buf-len:%d client close\n",
				__FILE__, __LINE__,
				_hostIp.c_str(), _hostPort, loop, len);
			return ERROR_RECV;
		}
		len += ret;
		//检查是否接受完
		ret = _func(buff, len);
		if (ret < 0)
		{
			snprintf(_errStr, 512, "[%s][%d]recv ip:%s port:%d loop:%d buf-len:%d server close\n",
				__FILE__, __LINE__,
				_hostIp.c_str(), _hostPort, loop, len);
			return ERROR_RECV;
		}

		if (ret == 0)
		{
			continue;
		}
		//接收数据完毕
		return SOCK_SUC;
	}
	snprintf(_errStr, 512, "[%s][%d]recv ip:%s port:%d loop:%d buf-len:%d error\n",
		__FILE__, __LINE__,
		_hostIp.c_str(), _hostPort, loop, len);
	return ERROR_RECV;
}

void SerSocket::SetPduProtocol()
{
	_func = PduProtocol;
}

int SerSocket::CheckRead(int fd)
{
	struct pollfd connectClient[1];
	int n = 1;
	memset(&connectClient[0], 0, sizeof(pollfd));
	connectClient[0].fd = fd;
	connectClient[0].events = POLLIN;

	int ret = 0;
	int loop = 5;
	for (int i = 0; i < loop; ++i)
	{
		ret = ::poll(connectClient, n, -1);
		//判断EINTR信号
		if (ret < 0 && errno == EINTR)
			continue;
		break;
	}
	return ret;
}

int SerSocket::CheckWrite(int fd)
{
	struct pollfd connectClient[1];
	int n = 1;
	memset(&connectClient[0], 0, sizeof(pollfd));
	connectClient[0].fd = fd;
	connectClient[0].events = POLLOUT;

	int ret = 0;
	int loop = 5;
	for (int i = 0; i < loop; ++i)
	{
		ret = ::poll(connectClient, n, 0);
		//判断EINTR信号
		if (ret < 0 && errno == EINTR)
			continue;
		break;
	}
	return ret;
}

int SerSocket::CheckReadAndWrite(int fd)
{
	struct pollfd connectClient[1];
	int n = 1;
	memset(&connectClient[0], 0, sizeof(pollfd));
	connectClient[0].fd = fd;
	connectClient[0].events = POLLIN | POLLOUT;

	int ret = 0;
	int loop = 5;
	for (int i = 0; i < loop; ++i)
	{
		ret = ::poll(connectClient, n, 0);
		//判断EINTR信号
		if (ret < 0 && errno == EINTR)
			continue;
		break;
	}
	return ret;
}

int SerSocket::SetNonBlock(int fd)
{
	int old = fcntl(fd, F_GETFL, 0);
	if (old == -1)
	{
		return ERROR_NONBLOCK;
	}
	if (fcntl(fd, F_SETFL, old | O_NONBLOCK | O_NDELAY) == -1)
	{
		return ERROR_NONBLOCK;
	}

	return SOCK_SUC;
}

int PduProtocol(const char* buff, int len)
{
	if (len < PDU_HEAD_SIZE)
	{
		return -1;
	}
	PDUHEAD* pHeader = (PDUHEAD*)buff;
	int pack_len = pHeader->GetPackLen();
	if (len < pack_len)
	{
		//没有接收完，继续接收
		return 0;
	}
	return pack_len;
}
