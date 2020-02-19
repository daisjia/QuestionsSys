#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <poll.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string>
#include <vector>
#include <map>
#include"pdu.h"
#include"IM.User.Msg.pb.h"
#include<pthread.h>
using namespace std;
int n;

void* func(void *)
{
	sleep(1);
	sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	saddr.sin_port = htons(6000);

	for (int i = 0; i < n; ++i)
	{
		if (i == 200)
		{
			i = 0;
		}
		int fd = socket(AF_INET, SOCK_STREAM, 0);
		connect(fd, (struct sockaddr*) & saddr, sizeof(saddr));
		int id = i;
		string passwd = "123456";
		int type = 0;
		PDUHEAD pReqHeader;
		pReqHeader.SetCmd(LOGIN);
		char ReqBuffer[1024 * 1024] = { 0 };
		IM::User::Msg::IMLoginReq Msg;
		Msg.set_id(id);
		Msg.set_passwd(passwd.c_str());
		Msg.set_type(type);
		std::string ReqMsg;
		Msg.SerializeToString(&ReqMsg);
		memcpy(ReqBuffer, (void*)&pReqHeader, pReqHeader.GetHeadLen());
		memcpy(ReqBuffer + pReqHeader.GetHeadLen(), ReqMsg.c_str(), ReqMsg.size());
		((PDUHEAD*)ReqBuffer)->SetPackLen(pReqHeader.GetHeadLen() + ReqMsg.size());

		send(fd, ReqBuffer, pReqHeader.GetHeadLen() + ReqMsg.size(), 0);

		char RspBuffer[1024 * 1024] = { 0 };
		recv(fd, RspBuffer, 1024 * 1024, 0);
		IM::User::Msg::IMRspMsg Msg2;
		PDUHEAD* pRspHeader = (PDUHEAD*)RspBuffer;
		std::string RspMsg = pRspHeader->GetBody();
		Msg2.ParseFromString(RspMsg);
		int ret = Msg2.ret();
		std::string msg = Msg2.msg();
		cout <<"++++++++++++++++" <<msg << endl;
		close(fd);
	}

}

int main()
{
	cout << "please input n: ";
	cin >> n;
	sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	saddr.sin_port = htons(6000);

	pthread_t id;
	pthread_create(&id, NULL, func, NULL);

	for (int i = 0; i < n; ++i)
	{
		int fd = socket(AF_INET, SOCK_STREAM, 0);

		connect(fd, (struct sockaddr*) & saddr, sizeof(saddr));
		char name[20];
		int id = i;
		string passwd = "123456";
		int type = 0;

		sprintf(name, "jiadai%d", i);

		PDUHEAD pReqHeader;
		pReqHeader.SetCmd(REGISTER);
		char ReqBuffer[1024 * 1024] = { 0 };
		IM::User::Msg::IMRegisterReq Msg;
		Msg.set_id(id);
		Msg.set_name(name);
		Msg.set_passwd(passwd.c_str());
		Msg.set_type(type);
		std::string ReqMsg;
		Msg.SerializeToString(&ReqMsg);
		memcpy(ReqBuffer, (void*)&pReqHeader, pReqHeader.GetHeadLen());
		memcpy(ReqBuffer + pReqHeader.GetHeadLen(), ReqMsg.c_str(), ReqMsg.size());
		((PDUHEAD*)ReqBuffer)->SetPackLen(pReqHeader.GetHeadLen() + ReqMsg.size());

		send(fd, ReqBuffer, pReqHeader.GetHeadLen() + ReqMsg.size(), 0);

		char RspBuffer[1024 * 1024] = { 0 };
		recv(fd, RspBuffer, 1024 * 1024, 0);

		IM::User::Msg::IMRspMsg Msg2;
		PDUHEAD* pRspHeader = (PDUHEAD*)RspBuffer;
		std::string RspMsg = pRspHeader->GetBody();
		Msg2.ParseFromString(RspMsg);
		int ret = Msg2.ret();
		std::string msg = Msg2.msg();
		cout << "-------------" << msg << endl;
		close(fd);
	}

	pthread_join(id, NULL);
}

