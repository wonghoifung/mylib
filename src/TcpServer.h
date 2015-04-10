#ifndef BOYAA_NET_H_20110313
#define BOYAA_NET_H_20110313


#ifdef WIN32
	#include <winsock2.h>
	#include <windows.h>
	#include <stdio.h>
	#define __INIT_NET_ENVIR__ int nError;WSADATA wsaData;if((nError=WSAStartup(MAKEWORD(2,2),&wsaData))!=0){return false;}
#else
	#include <sys/resource.h>
	#include <sys/times.h>
	#include <sys/socket.h>
	#include <sys/types.h>
	#include <sys/epoll.h>
	#include <sys/ioctl.h>
	#include <arpa/inet.h>
	#include <netinet/in.h>
	#include <errno.h>
	#include <fcntl.h>
	#include <unistd.h>
	typedef int SOCKET;
	#define INVALID_SOCKET -1
	#define SOCKET_ERROR -1
	#define WSAGetLastError() errno
	#define	__INIT_NET_ENVIR__
#endif // WIN32

#include <errno.h>
#include <map>
#include <vector>

using namespace std;

#include "TcpHandler.h"
#include "common.h"

// 网络模块类
class TcpServer
{
public:
	TcpServer();
	virtual ~TcpServer();
public:

#ifndef WIN32
    //静态函数，处理关闭信号
    static void SigHandle(int signum);
#endif

	// 初始化监听
	bool                    InitSocket(int listen_port);	
	bool                    Run();
    
	// 动态创建Handler
	virtual TcpHandler *    CreateHandler(void)                 = 0;
public:	
	TcpHandler*             AllocSocketHandler(SOCKET sock_fd);
	// 注册一个handle
	bool                    Register(TcpHandler *pHandler);
	// 主动断开连接
	bool                    DisConnect(TcpHandler * pSocketHandler);

    void                    WantWrite(TcpHandler * s);
    void                    WantRead(TcpHandler * s);
protected:
	int                     handle_accept();
    void                    AddSocket(TcpHandler * s);
    void                    RemoveSocket(TcpHandler * s);

private:
	bool                    _InitEvent();
	bool                    _StartUpLin();
	bool                    _StartUpWin();
    void                    handle_close(TcpHandler* pHandler);

protected:
    static bool         m_bRun;
    SOCKET	            m_listen_fd;
	int		            m_maxfd;
    TcpHandler **fds;    
    int                 m_count_fd;
    uint32              m_fd_index;         // 分配句柄计数
#ifdef WIN32   
	fd_set	    m_rset;    
	fd_set	    m_wset;
	fd_set      m_tmp_rset; 
	fd_set      m_tmp_wset;
#else
	int         m_epoll_fd;
	struct epoll_event* m_epev_arr; //epoll_event数组	
#endif
};
#endif

