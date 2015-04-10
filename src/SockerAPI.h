
/***************************************************************
			CSocker socketͨѶ��
		��̬�࣬��socketͨѶ�Ĳ����ṩ�˷�װ�������˴�����
***************************************************************/

#ifndef __SOCKER_API_H__
#define __SOCKER_API_H__

#ifndef WIN32
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>
#else
#include <WinSock2.h>
#include <Windows.h>
#include <WinSock.h>

#endif

//stl
#include <vector>
#include <iostream>
#include <string>

#include "log.h"

using namespace std;
#define MAX_LISTEN_QUEUE 100000

struct NetAddr
{
	int     port;
	string  host;
    NetAddr(){
        host = "";
        port = 0;
    }
	NetAddr(const NetAddr &addr){
        host = addr.host;
        port = addr.port;}
	NetAddr & operator=(const NetAddr& addr){
		host = addr.host;
        port=addr.port;return *this;
	}
};
//״̬λ
enum STATUS
{
    CONNECT=0, 
    REQUEST, 
    CLOSE
};

class CSocker
{
public:
	static vector<int> sessions;
	static int maxnums;

public:
	CSocker(void);
public:
	~CSocker(void);
public:
	//����˵ļ���������һ���������ͼ����Ķ˿�
	static int ServerListen(int fd , int port);
public:
	//����˽������󣬴������˵�������
	static int ServerAccept(int fd);
public:
	//socket������Ϣ������������������������С
	static int SocketSend(int fd, const char* buf, size_t len);
public:
	//socket������Ϣ������������������������С ....... ����
	static int SocketRecv(int fd , void* buf , size_t len );
public:
	// �ر�socket�׽���
	static void SocketClose(int fd);    
public:
	//�ͻ���socket����ָ��ip�Ͷ˿ڵķ�����
	static int ClientConnect(int fd, const char* ip , int port );
public:
	//����socketΪ������ģʽ
	static int SocketNoBlock(int fd);
public:
	//�˿ڿ�����
	static int SocketReUse(int fd);
public:
    //����TCP����
    static int SetTcpKeepLive(int fd);
public:
	//��ʼ��һ��socket
	static int SocketInit(void);
public:
    // �����շ���������С
    static void SetSocketMem(int fd,int iSize);
public:
    // ���������ӵ�Զ��
	static int ConnNoblock(int fd,const char* ip, int port);
public:
	static int WaitForConnect(int seconds);
};

#endif
