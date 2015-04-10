#include "SockerAPI.h"

#ifdef WIN32
#include <MSTcpIP.h>
#else
#include <netinet/tcp.h>
#include <netinet/ip.h>
#endif


vector<int> CSocker::sessions;		//����
int CSocker::maxnums = 0;

CSocker::CSocker(void)
{
}

CSocker::~CSocker(void)
{
}


/*
*����:
		int CSocker::ServerListen(int fd, int port)
*���ܣ�
		�󶨲������˿�
*���������
		fd 	�׽���������
		portҪ�����Ķ˿�
*���������
		��
*����ֵ��	
		0  �ɹ�
		-1 ����
*/
int CSocker::ServerListen(int fd, int port)
{
	//���SocketΪ����IP�Ͷ˿�
	struct sockaddr_in addr;
	memset( &addr , 0,sizeof(addr) );
	addr.sin_family			= AF_INET;
	addr.sin_port			= htons(port);
	addr.sin_addr.s_addr	= htonl(INADDR_ANY);//�Զ���䱾��IP

	//���׽���
	if ( 0 != bind( fd , (struct sockaddr *)&addr , sizeof(addr) ) )
	{
		//������׽���ʧ��
		log_error("Error: Bind Faile bind() %s\n",strerror(errno));		
		return -1;		
	}

	//������fd������������ΪMAX_LISTEN_QUEUE
	if( 0 != listen( fd, MAX_LISTEN_QUEUE ) )
	{
		//�������ʧ��
		log_error("Error: Listen Faile listen(): %s\n", strerror(errno));		
		return -1;		
	}   

	return 0;
}


/*
*����:
		int CSocker::ServerAccept(int fd)
*���ܣ�
		����һ����������
*���������
		fd 	��������fd
*���������
		��
*����ֵ��	
		-1 ����
		�ɹ��Ļ����ؽ��յ���fd
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

	//����������һ������
	connfd = accept( fd , (struct sockaddr *)&clientaddr , &clilen );
	//���������Ӵ���
	if( connfd < 0 )
	{
		return -1;
	}
	else
	{
		//�������ӹ������׽���������
        log_debug("connection from %s ,port %d \n",inet_ntoa(clientaddr.sin_addr),ntohs(clientaddr.sin_port));
		return connfd;
	}
}


/*
*����:
		int CSocker::SocketSend(int fd, void* buf, size_t len)
*���ܣ�
		�������ݵ���Ӧ��fd
*���������
		fd 	Ҫ���͵��׽���������
		buf Ҫ���͵Ļ�������ַ
		len	Ҫ���͵��ֽڳ���
*���������
		��
*����ֵ��	
		<=0 ���ʹ���
		>0  �ɹ����͵��ֽ���
*/
int CSocker::SocketSend(int fd, const char* buf, size_t len)
{
	int ret	= send( fd, buf, len, 0);
    return ret;
}

/*
*����:
		int CSocker::SocketRecv(int fd, void* buf, size_t len)
*���ܣ�
		�������ݵ���Ӧ��fd
*���������
		fd 	Ҫ���յ��׽���������
		buf Ҫ���յĻ�������ַ
		len	Ҫ���յĻ�������С
*���������
		��
*����ֵ��	
		<=0 ���ʹ���
		>0  �ɹ����͵��ֽ���
*/
int CSocker::SocketRecv(int fd , void* buf , size_t len )
{
	//ʵ�ʽ����˶����ֽ�
	int ret = recv( fd, (char*)buf, len, 0 );
    return ret;
}

/*
*����:
		void CSocker::SocketClose( int fd )
*���ܣ�
		�ر��׽���
*/
void CSocker::SocketClose( int fd )
{
    //if(bSendOver)
    //{
    //    //����Ĵ�����һ��ʹ��SO_LINGERѡ�ʹ��30��ĳ�ʱʱ�ޣ�
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
*����:
		int CSocker::ClientConnect(int fd, const char* ip , int port )
*���ܣ�
		�������ӵ�ָ����ip�Ͷ˿�
*���������
		fd 	Ҫ���ӵ��׽���������
		ip 	Ҫ���ӵ�IP��ַ
		portҪ���ӵĶ˿�
*���������
		��
*����ֵ��	
		0  ���ӳɹ�
		-1 Connect����
*/
//��Ȼ�󷵻ظ�socket�����ӷ���ʧ�ܷ���-1
int CSocker::ClientConnect(int fd, const char* ip , int port )
{
	//TCP  Socket

	//��ʼ��Զ�������ṹ
	struct sockaddr_in remote;
	memset( &remote, 0,sizeof(remote));
	remote.sin_family 		 = AF_INET;
	remote.sin_port   		 = htons(port);
	remote.sin_addr.s_addr = inet_addr(ip);

	//����Զ�̷�����
	if(0 != connect(fd, (struct sockaddr*)&remote, sizeof(remote)))
	{
		log_error("Error: Connect Faile connect(): %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

/*
*����:
		int CSocker::SocketNoBlock(int fd)
*���ܣ�
		����Socket�׽��ַ�����
*���������
		fd 	Ҫ���õ��׽���������
*���������
		��
*����ֵ��	
		0  �ɹ�
		-1 ����
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
*����:
		int CSocker::SocketReUse(int fd)
*���ܣ�
		���ö˿ڿ�����
*���������
		��
*����ֵ��	
		0  �ɹ�
		-1 ʧ��
*/
//���ö˿ڿ�����
int CSocker::SocketReUse(int fd)
{
	int opt = 1;
	return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));	
}
//����TCP����
int CSocker::SetTcpKeepLive( int fd )
{

#ifdef WIN32
    struct tcp_keepalive keepAlive = {0};
    keepAlive.onoff = 1;
    keepAlive.keepaliveinterval = 5000;   //��λΪ����
    keepAlive.keepalivetime = 1000;       //��λΪ����

    return setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&keepAlive, sizeof(keepAlive));
#else
    int keepAlive = 1; // ����keepalive����
    int keepIdle = 60; // ���������60����û���κ���������,�����̽�� 
    int keepInterval = 1; // ̽��ʱ������ʱ����Ϊ1 ��
    int keepCount = 3;  // ̽�Ⳣ�ԵĴ���.�����1��̽������յ���Ӧ��,���2�εĲ��ٷ�.

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
*����:
		int CSocker::SocketInit(void)
*���ܣ�
		����һ��Socket�׽���
*����ֵ��	
		�ɹ�����Socket fd
		-1 ����ʧ��
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
// �����շ���������С
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
*����:
		ConnNoblock(int fd, const char* ip, int port)
*���ܣ�
		����һ��������������
		(��fd��ӵ�CSocker���һ��vector<int>��̬��Ա����CSocker::sessions)
*���������
		fd 	Ҫ���ӵ��׽���������
		ip 	Ҫ���ӵ�IP��ַ
		portҪ���ӵĶ˿�
*���������
		��
*����ֵ��	
		0  �ɹ���ӵ��ȴ�����
		-1 Connect����
*/

int CSocker::ConnNoblock(int fd, const char* ip, int port)
{
	//��ʼ��Զ�������ṹ
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
*����:
		WaitForConnect(int seconds)
*���ܣ�
		�ȴ�����������������
		(��CSocker���һ��vector<int>��̬��Ա����CSocker::sessions)
*���������
		seconds �ȴ��ĳ�ʱ��ʱ��Ϊ����
*���������
		��
*����ֵ��	
		0  �ȴ��Ѿ���ʱ�����������Ѿ��������
		-1 Select��������
		>0 ����һ���ɹ������ϵ�fd����fd����
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
	
	//��ʼ��select
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
	else if(n < 0)	//���С����
	{
		log_debug("Error: Select Error!!!\n");
		return -1;
	}
	
	for(iter = CSocker::sessions.begin(); iter != CSocker::sessions.end();)
	{
		if(FD_ISSET(*iter, &readset) || FD_ISSET(*iter, &writeset))
		{
			//�������
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
			//��ʱ�Ѿ������˲������˸õ��������������õ���������
			continue;
		}
		
		++iter;
	}

	return 0;
}
