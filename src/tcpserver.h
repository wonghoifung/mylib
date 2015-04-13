#ifndef TCPSERVER_HEADER
#define TCPSERVER_HEADER

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

#include <errno.h>
#include <map>
#include <vector>

using namespace std;

#include "tcphandler.h"
#include "common.h"

// ����ģ����
class TcpServer
{
public:
	TcpServer();
	virtual ~TcpServer();
public:

#ifndef WIN32
    //��̬����������ر��ź�
    static void SigHandle(int signum);
#endif

	// ��ʼ������
	bool                    InitSocket(int listen_port);	
	bool                    Run();
    
	// ��̬����Handler
	virtual TcpHandler *    CreateHandler(void)                 = 0;
public:	
	TcpHandler*             AllocSocketHandler(SOCKET sock_fd);
	// ע��һ��handle
	bool                    reg(TcpHandler *pHandler);
	// �����Ͽ�����
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
    uint32              m_fd_index;         // ����������
#ifdef WIN32   
	fd_set	    m_rset;    
	fd_set	    m_wset;
	fd_set      m_tmp_rset; 
	fd_set      m_tmp_wset;
#else
	int         m_epoll_fd;
	struct epoll_event* m_epev_arr; //epoll_event����	
#endif
};
#endif

