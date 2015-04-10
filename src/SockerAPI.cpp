#include "SockerAPI.h"

#ifdef WIN32
#include <MSTcpIP.h>
#else
#include <netinet/tcp.h>
#include <netinet/ip.h>
#endif


vector<int> CSocker::sessions;		//用于
int CSocker::maxnums = 0;

CSocker::CSocker(void)
{
}

CSocker::~CSocker(void)
{
}


/*
*名称:
		int CSocker::ServerListen(int fd, int port)
*功能：
		绑定并监听端口
*输入参数：
		fd 	套接字描述符
		port要监听的端口
*输出参数：
		无
*返回值：	
		0  成功
		-1 出错
*/
int CSocker::ServerListen(int fd, int port)
{
	//填充Socket为本地IP和端口
	struct sockaddr_in addr;
	memset( &addr , 0,sizeof(addr) );
	addr.sin_family			= AF_INET;
	addr.sin_port			= htons(port);
	addr.sin_addr.s_addr	= htonl(INADDR_ANY);//自动填充本机IP

	//绑定套接字
	if ( 0 != bind( fd , (struct sockaddr *)&addr , sizeof(addr) ) )
	{
		//如果绑定套接字失败
		log_error("Error: Bind Faile bind() %s\n",strerror(errno));		
		return -1;		
	}

	//监听该fd，最大监听数量为MAX_LISTEN_QUEUE
	if( 0 != listen( fd, MAX_LISTEN_QUEUE ) )
	{
		//如果监听失败
		log_error("Error: Listen Faile listen(): %s\n", strerror(errno));		
		return -1;		
	}   

	return 0;
}


/*
*名称:
		int CSocker::ServerAccept(int fd)
*功能：
		接收一个连接请求
*输入参数：
		fd 	服务器的fd
*输出参数：
		无
*返回值：	
		-1 出错
		成功的话返回接收到的fd
*/
int CSocker::ServerAccept(int fd)
{
	struct sockaddr_in clientaddr;
#ifdef WIN32
	int clilen ;
#else
	socklen_t clilen;
#endif
	clilen = sizeof(clientaddr);
	int connfd;

	//服务器接收一个连接
	connfd = accept( fd , (struct sockaddr *)&clientaddr , &clilen );
	//如果这个连接错误
	if( connfd < 0 )
	{
		return -1;
	}
	else
	{
		//返回连接过来的套接字描述符
        log_debug("connection from %s ,port %d \n",inet_ntoa(clientaddr.sin_addr),ntohs(clientaddr.sin_port));
		return connfd;
	}
}


/*
*名称:
		int CSocker::SocketSend(int fd, void* buf, size_t len)
*功能：
		发送数据到对应的fd
*输入参数：
		fd 	要发送的套接字描述符
		buf 要发送的缓冲区地址
		len	要发送的字节长度
*输出参数：
		无
*返回值：	
		<=0 发送错误
		>0  成功发送的字节数
*/
int CSocker::SocketSend(int fd, const char* buf, size_t len)
{
	int ret	= send( fd, buf, len, 0);
    return ret;
}

/*
*名称:
		int CSocker::SocketRecv(int fd, void* buf, size_t len)
*功能：
		发送数据到对应的fd
*输入参数：
		fd 	要接收的套接字描述符
		buf 要接收的缓冲区地址
		len	要接收的缓冲区大小
*输出参数：
		无
*返回值：	
		<=0 发送错误
		>0  成功发送的字节数
*/
int CSocker::SocketRecv(int fd , void* buf , size_t len )
{
	//实际接收了多少字节
	int ret = recv( fd, (char*)buf, len, 0 );
    return ret;
}

/*
*名称:
		void CSocker::SocketClose( int fd )
*功能：
		关闭套接字
*/
void CSocker::SocketClose( int fd )
{
    //if(bSendOver)
    //{
    //    //下面的代码是一个使用SO_LINGER选项，使用30秒的超时时限：
    //    int z; /* Status code*/ 
    //    struct linger so_linger;
    //    so_linger.l_onoff = 1;
    //    so_linger.l_linger = 30;
    //    z = setsockopt(fd,SOL_SOCKET,SO_LINGER,(char*)&so_linger,sizeof(so_linger));
    //    if(z)
    //        log_error("safe close socket error \n");
    //}
#ifdef WIN32
	closesocket(fd);
#else
	close(fd);
#endif
}

/*
*名称:
		int CSocker::ClientConnect(int fd, const char* ip , int port )
*功能：
		阻塞连接到指定的ip和端口
*输入参数：
		fd 	要连接的套接字描述符
		ip 	要连接的IP地址
		port要连接的端口
*输出参数：
		无
*返回值：	
		0  连接成功
		-1 Connect出错
*/
//，然后返回该socket的连接符，失败返回-1
int CSocker::ClientConnect(int fd, const char* ip , int port )
{
	//TCP  Socket

	//初始化远程主机结构
	struct sockaddr_in remote;
	memset( &remote, 0,sizeof(remote));
	remote.sin_family 		 = AF_INET;
	remote.sin_port   		 = htons(port);
	remote.sin_addr.s_addr = inet_addr(ip);

	//连接远程服务器
	if(0 != connect(fd, (struct sockaddr*)&remote, sizeof(remote)))
	{
		log_error("Error: Connect Faile connect(): %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

/*
*名称:
		int CSocker::SocketNoBlock(int fd)
*功能：
		设置Socket套接字非阻塞
*输入参数：
		fd 	要设置的套接字描述符
*输出参数：
		无
*返回值：	
		0  成功
		-1 出错
*/
int CSocker::SocketNoBlock(int fd)
{    

#ifdef WIN32
	unsigned long argp = 1;
	ioctlsocket(fd, FIONBIO, &argp);
	bool bNoDelay = true;
	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&bNoDelay, sizeof(bNoDelay));
#else
	int opts = fcntl(fd, F_GETFL);
	if( opts < 0 )
		return -1;
	opts = opts | O_NONBLOCK;
	if( fcntl(fd, F_SETFL, opts) < 0)
		return -1;
#endif

	return 0;
}

/*
*名称:
		int CSocker::SocketReUse(int fd)
*功能：
		设置端口可重用
*输出参数：
		无
*返回值：	
		0  成功
		-1 失败
*/
//设置端口可重用
int CSocker::SocketReUse(int fd)
{
	int opt = 1;
	return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));	
}
//开启TCP心跳
int CSocker::SetTcpKeepLive( int fd )
{

#ifdef WIN32
    struct tcp_keepalive keepAlive = {0};
    keepAlive.onoff = 1;
    keepAlive.keepaliveinterval = 5000;   //单位为毫秒
    keepAlive.keepalivetime = 1000;       //单位为毫秒

    return setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&keepAlive, sizeof(keepAlive));
#else
    int keepAlive = 1; // 开启keepalive属性
    int keepIdle = 60; // 如该连接在60秒内没有任何数据往来,则进行探测 
    int keepInterval = 1; // 探测时发包的时间间隔为1 秒
    int keepCount = 3;  // 探测尝试的次数.如果第1次探测包就收到响应了,则后2次的不再发.

    int ret1 = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive, sizeof(keepAlive));
    int ret2 = setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle));
    int ret3 = setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
    int ret4 = setsockopt(fd, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount)); 

    if(ret1 == 0 && ret2 == 0 && ret3 == 0 && ret4 == 0)
        return 0;
    else
        return -1;
#endif
}

/*
*名称:
		int CSocker::SocketInit(void)
*功能：
		创建一个Socket套接字
*返回值：	
		成功返回Socket fd
		-1 创建失败
*/
int CSocker::SocketInit(void)
{
	int fd = socket(AF_INET , SOCK_STREAM , 0);

	if( 0 > fd)
	{
		log_error("Error  socket() %s\n",strerror(errno));		
	}
	return fd;
}
// 设置收发缓冲区大小
void CSocker::SetSocketMem(int fd,int iSize)
{
#ifndef WIN32
    int opt = iSize;
    socklen_t optlen = sizeof(opt);
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &opt, optlen);
    setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &opt, optlen);
#endif     
}


/*
*名称:
		ConnNoblock(int fd, const char* ip, int port)
*功能：
		发送一个非阻塞的连接
		(将fd添加到CSocker类的一个vector<int>静态成员变量CSocker::sessions)
*输入参数：
		fd 	要连接的套接字描述符
		ip 	要连接的IP地址
		port要连接的端口
*输出参数：
		无
*返回值：	
		0  成功添加到等待链表
		-1 Connect出错
*/

int CSocker::ConnNoblock(int fd, const char* ip, int port)
{
	//初始化远程主机结构
	struct sockaddr_in remote;
	memset( &remote, 0,sizeof(remote));
	remote.sin_family 		 = AF_INET;
	remote.sin_port   		 = htons(port);
	remote.sin_addr.s_addr = inet_addr(ip);
	
	if(fd > 0)
	{
		if(0 == CSocker::SocketNoBlock(fd))
		{
			if(0 > connect(fd, (struct sockaddr*)&remote, sizeof(remote)))
			{	
#ifdef WIN32
				return -1;
#else
				if(errno != EINPROGRESS)
				{	
					return -1;
				}  
#endif 
			}
			CSocker::sessions.push_back(fd);
			CSocker::maxnums++;
			return 0;
		}
		else
		{
			log_debug("Error NoBlock Faile Fd %d \n", fd);
		}
	}
	else
	{
		log_debug("Error Fd %d \n", fd);
	}
	return -1;
}


/*
*名称:
		WaitForConnect(int seconds)
*功能：
		等待批量非阻塞的连接
		(由CSocker类的一个vector<int>静态成员变量CSocker::sessions)
*输入参数：
		seconds 等待的超时，时间为秒数
*输出参数：
		无
*返回值：	
		0  等待已经超时，或者连接已经处理完成
		-1 Select函数出错
		>0 捕获到一个成功连接上的fd，把fd返回
*/
int CSocker::WaitForConnect(int seconds)
{
	if(CSocker::maxnums <= 0)
	{
		log_debug("Notice: Finish\n");
		CSocker::maxnums = 0;
		return 0;
	}
	
	int maxfd = 0;
	int error = 0;
#ifdef WIN32
	int errlen = sizeof(error);
#else
	socklen_t errlen = sizeof(error);
#endif
	
	fd_set readset;
	fd_set writeset;	
	
	FD_ZERO(&readset);
	FD_ZERO(&writeset);
	
	//初始化select
	vector<int>::iterator iter = CSocker::sessions.begin();
	for(; iter != CSocker::sessions.end(); ++iter)
	{
		FD_SET(*iter, &readset);
		FD_SET(*iter, &writeset);
		if(maxfd < *iter)
		{
			maxfd = *iter;
		}
	}
	
	struct timeval tval;
	tval.tv_sec = seconds;
	tval.tv_usec = 0;
	
	int n = 0;
	if(0 == (n = select(maxfd + 1, &readset, &writeset, NULL, seconds ? &tval : NULL)))
	{
	
		for(iter = CSocker::sessions.begin(); iter != CSocker::sessions.end(); ++iter)
		{
			SocketClose(*iter);
		}		
		log_debug("Notice: Select Time Out!!!\n");
		CSocker::sessions.clear();
		CSocker::maxnums = 0;
		return 0;
	}
	else if(n < 0)	//如果小于零
	{
		log_debug("Error: Select Error!!!\n");
		return -1;
	}
	
	for(iter = CSocker::sessions.begin(); iter != CSocker::sessions.end();)
	{
		if(FD_ISSET(*iter, &readset) || FD_ISSET(*iter, &writeset))
		{
			//如果错误
			if(getsockopt(*iter, SOL_SOCKET, SO_ERROR, (char*)&error, &errlen) < 0)
			{
				log_debug("Error: Can not Connect To Server Ad Fd %d\n", *iter);
				iter = CSocker::sessions.erase(iter);
				--CSocker::maxnums;
				SocketClose(*iter);
			}
			else
			{
				int rtfd = *iter;
				iter = CSocker::sessions.erase(iter);
				--CSocker::maxnums;
				if(error != 0)
				{
					log_debug("Error: Connect To Server Ad Fd %d Faile %s\n", *iter, strerror(error));
				}
				else
				{
					log_debug("Notice: Connect To Server Ad Fd %d Success \n", *iter);
					return rtfd;
				}				
			}
			//此时已经处理了并更新了该迭代器，不可以让迭代器自增
			continue;
		}
		
		++iter;
	}

	return 0;
}
