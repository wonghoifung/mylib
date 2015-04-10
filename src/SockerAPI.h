
/***************************************************************
			CSocker socket通讯类
		静态类，对socket通讯的操作提供了封装，进行了错误处理
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
//状态位
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
	//服务端的监听，传入一个描述符和监听的端口
	static int ServerListen(int fd , int port);
public:
	//服务端接收请求，传入服务端的描述符
	static int ServerAccept(int fd);
public:
	//socket发送消息，传入描述符，缓冲区，大小
	static int SocketSend(int fd, const char* buf, size_t len);
public:
	//socket接收消息，传入描述符，缓冲区，大小 ....... ！！
	static int SocketRecv(int fd , void* buf , size_t len );
public:
	// 关闭socket套接字
	static void SocketClose(int fd);    
public:
	//客户端socket连接指定ip和端口的服务器
	static int ClientConnect(int fd, const char* ip , int port );
public:
	//设置socket为非阻塞模式
	static int SocketNoBlock(int fd);
public:
	//端口可重用
	static int SocketReUse(int fd);
public:
    //开启TCP心跳
    static int SetTcpKeepLive(int fd);
public:
	//初始化一个socket
	static int SocketInit(void);
public:
    // 设置收发缓冲区大小
    static void SetSocketMem(int fd,int iSize);
public:
    // 非阻塞连接到远端
	static int ConnNoblock(int fd,const char* ip, int port);
public:
	static int WaitForConnect(int seconds);
};

#endif
