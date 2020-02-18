#include "cli_socket.h"

static PDUHEAD h;
static int PDU_HEAD_SIZE = h.GetHeadLen();
CliSocket* CliSocket::cli = NULL;
std::mutex CliSocket::lck;

CliSocket::CliSocket()
{
	_sockfd = -1;
	_func = NULL;
	_hostIp = "";
	_hostPort = 0;
	SetPduProtocol();
}

CliSocket::~CliSocket()
{
	Close();
	_func = NULL;
}

int CliSocket::Init(const char* ip, int port, bool keep_alive)
{
	_hostIp = ip;
	_hostPort = port;
	_keepAlive = keep_alive;
	bzero(&_hostAddr, sizeof(_hostAddr));
	_hostAddr.sin_family = AF_INET;
	inet_aton(_hostIp.c_str(), &_hostAddr.sin_addr);
	_hostAddr.sin_port = htons(_hostPort);
	return 0;
}

int CliSocket::Close()
{
	if (_sockfd > 0)
	{
		close(_sockfd);
		_sockfd = -1;
	}
	return SOCK_SUC;
}

void CliSocket::GetHost(std::string& ip, int& port)
{
	ip = _hostIp;
	port = _hostPort;
}

int CliSocket::Connect()
{
	CHECK_HOSP();
	_errStr[0] = '0';

	int ret = 0;
	if (_sockfd > 0)
	{
		ret = CheckRead(_sockfd);
		if (ret > 0)
			Close();
		else
			return _sockfd;
	}

	_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (_sockfd <= 0)
	{
		snprintf(_errStr, 512, "[%s][%d]create socket ip:%s port:%d err:%s\n",
			__FILE__, __LINE__,
			_hostIp.c_str(), _hostPort, strerror(errno));
		Close();
		exit(0);
	}

	if (SetNonBlock(_sockfd) < 0)
	{
		snprintf(_errStr, 512, "[%s][%d]fcntl set ip:%s port:%d non-block error\n",
			__FILE__, __LINE__,
			_hostIp.c_str(), _hostPort);
		Close();
		exit(0);
	}

	ret = ::connect(_sockfd, (struct sockaddr*)&_hostAddr, sizeof(_hostAddr));
	if(ret < 0)
	{
		//非阻塞模式下返回-1，error == EINPROGRESS 不能证明连接失败,表示正在连接，后边继续监测
		if (errno != EINPROGRESS)
		{
			snprintf(_errStr, 512, "[%s][%d]asyncconnect ip:%s port:%d error:%s\n",
				__FILE__, __LINE__,
				_hostIp.c_str(), _hostPort, strerror(errno));
			Close();
			return ERROR_CONNECT;
		}
	}
	else if (ret == 0)
	{
		//connect 返回0为连接成功
		return _sockfd;
	}

	//返回-1，error == EINPROGRESS 继续判断
	struct pollfd connectClient[1];
	int n = 1;
	memset(&connectClient[0], 0, sizeof(pollfd));
	connectClient[0].fd = _sockfd;
	connectClient[0].events = POLLIN | POLLOUT;

	ret = 0;
	int loop = 5;
	for (int i = 0; i < loop; ++i)
	{
		ret = ::poll(connectClient, n, 0);
		//判断EINTR信号
		if (ret < 0 && errno == EINTR)
			continue;
		break;
	}

	//连接超时，表示失败
	if (ret <= 0)
	{
		snprintf(_errStr, 512, "[%s][%d]asyncconnect ip:%s port:%d _ret:%d err:%s\n",
			__FILE__, __LINE__,
			_hostIp.c_str(), _hostPort, ret, strerror(errno));
		Close();
		return ERROR_CONNECT;
	}
	else
	{
		//成功返回可写事件，失败返回可读可写事件
		if ((connectClient[0].revents & POLLIN) || (connectClient[0].revents & POLLOUT))
		{
			int error;
			int len = sizeof(error);
			int bok = getsockopt(_sockfd, SOL_SOCKET, SO_ERROR, &error, (socklen_t*)&len);
			if (bok < 0)
			{
				snprintf(_errStr, 512, "[%s][%d]asyncconnect ip:%s port:%d _ret:%d err:%s\n",
					__FILE__, __LINE__,
					_hostIp.c_str(), _hostPort, ret, strerror(errno));
				Close();
				return ERROR_CONNECT;
			}
			else if (error)
			{
				snprintf(_errStr, 512, "[%s][%d]asyncconnect ip:%s port:%d _ret:%d err:%s\n",
					__FILE__, __LINE__,
					_hostIp.c_str(), _hostPort, ret, strerror(errno));
				Close();
				return ERROR_CONNECT;
			}
			return _sockfd;
		}
		else if ((connectClient[0].revents & POLLERR) || (connectClient[0].revents & POLLHUP) || (connectClient[0].revents & POLLNVAL))
		{
			snprintf(_errStr, 512, "[%s][%d]asyncconnect ip:%s port:%d _ret:%d err:%s\n",
				__FILE__, __LINE__,
				_hostIp.c_str(), _hostPort, ret, strerror(errno));
			Close();
			return ERROR_CONNECT;
		}
	}
}

int CliSocket::SendBuf(char *message, int len)
{
	CHECK_ALL();
	_errStr[0] = '\0';

	int left = len;
	int nsend = 0;
	int loop = 1000;
	for (int i = 0; i < loop; ++i)
	{
		/*if (CheckWrite(_sockfd) <= 0)
		{
			snprintf(_errStr, 512, "[%s][%d]recv ip:%s port:%d loop:%d buf-len:%d error\n",
				__FILE__, __LINE__,
				_hostIp.c_str(), _hostPort, loop, len);
			Close();
			return ENUM_SOCKRSP_ERR_SEND;
		}*/

		int ret = send(_sockfd, message + nsend, left, 0);
		if (ret < 0)
		{
			if (errno == EINTR || errno == EAGAIN)
			{
				continue;
			}
			snprintf(_errStr, 512, "[%s][%d]recv ip:%s port:%d loop:%d buf-len:%d error\n",
				__FILE__, __LINE__,
				_hostIp.c_str(), _hostPort, loop, len);
			Close();
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
		Close();
		return ERROR_SEND;
	}
	return SOCK_SUC;
}

int CliSocket::RecvBuf(char * buff, int needlen, int& recvlen)
{
	//char buff[1024 * 1024]{ 0 };
	CHECK_ALL();
	_errStr[0] = '\0';
	int len = 0;
	int loop = 1000;
	for (int i = 0; i < loop; ++i)
	{
		if (CheckRead(_sockfd) <= 0)
		{
			snprintf(_errStr, 512, "[%s][%d]recv ip:%s port:%d loop:%d buf-len:%d error\n",
				__FILE__, __LINE__,
				_hostIp.c_str(), _hostPort, loop, len);
			Close();
			return ERROR_RECV;
		}

		if ((needlen - len) <= 0)
		{
			snprintf(_errStr, 512, "[%s][%d]recv ip:%s port:%d loop:%d buf-len:%d error\n",
				__FILE__, __LINE__,
				_hostIp.c_str(), _hostPort, loop, len);
			Close();
			return ERROR_RECV;
		}


		int ret = recv(_sockfd, buff + len, (needlen - len), 0);
		if (ret < 0)
		{
			if (errno == EINTR || errno == EAGAIN)
			{
				continue;
			}
			snprintf(_errStr, 512, "[%s][%d]recv ip:%s port:%d loop:%d buf-len:%d error\n",
				__FILE__, __LINE__,
				_hostIp.c_str(), _hostPort, loop, len);
			Close();
			return ERROR_RECV;
		}
		if (ret == 0)
		{
			snprintf(_errStr, 512, "[%s][%d]recv ip:%s port:%d loop:%d buf-len:%d server close\n",
				__FILE__, __LINE__,
				_hostIp.c_str(), _hostPort, loop, len);
			Close();
			exit(0);
		}
		len += ret;
		ret = _func(buff, len);
		if (ret < 0)
		{
			snprintf(_errStr, 512, "[%s][%d]recv ip:%s port:%d loop:%d buf-len:%d server close\n",
				__FILE__, __LINE__,
				_hostIp.c_str(), _hostPort, loop, len);
			Close();
			return ERROR_RECV;
		}

		if (ret == 0)
		{
			continue;
		}

		if (!_keepAlive)
		{
			Close();
		}
		//接收数据完毕
		return SOCK_SUC;
	}
	snprintf(_errStr, 512, "[%s][%d]recv ip:%s port:%d loop:%d buf-len:%d error\n",
		__FILE__, __LINE__,
		_hostIp.c_str(), _hostPort, loop, len);
	Close();
	return ERROR_RECV;
}

void CliSocket::SetPduProtocol()
{
	_func = PduProtocol;
}

int CliSocket::CheckRead(int fd)
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

int CliSocket::CheckWrite(int fd)
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

int CliSocket::CheckReadAndWrite(int fd)
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

int CliSocket::SetNonBlock(int fd)
{
	int old = fcntl(fd, F_GETFL, 0);
	if (old == -1)
	{
		return -1;
	}
	if (fcntl(fd, F_SETFL, old | O_NONBLOCK | O_NDELAY) == -1)
	{
		return -1;
	}

	return 0;
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
		return 0;
	}
	return pack_len;
}
