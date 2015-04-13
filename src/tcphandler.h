#ifndef TCPHANDLER_HEADER
#define TCPHANDLER_HEADER

#define RECV_BUFFER 4096

#include "timerwrapper.h"
#include "timerhandler.h"
#include <map>
#include "loopbuf.h"

using namespace std;
class tcpserver;
class inpack1;

const int MAX_LOOP_BUFFER_LEN = 64*1024; // 32k

#define  RECV_BUFFER_SIZE (1024*32)		//8k的缓冲区
#define  SEND_BUFFER_SIZE (1024*32)		//3k的缓冲区


class tcphandler : public TimerOutEvent
{
public:
	tcphandler();	
	virtual ~tcphandler();	

	void	    setfd(int sock_fd);//sock_fd
	int		    getfd()const;//sock_fd

    uint32      get_fd_index(){ return fdindex_;}
    void        set_fd_index(uint32 index){ fdindex_ = index;}
  
    bool        getneeddel(){ return m_bNeedDel; }
    void        setneeddel(bool bDel){ m_bNeedDel = bDel; }
public:
	int		    handle_connected();
	int		    handle_read();
	int		    handle_output();
	int		    handle_close();
protected:
	// 自定义Hook 函数
	virtual int onclose(void)					{return -1;}	// 连接断开后调用
	virtual int onconnected(void)				{return 0;}	    // 连接成功建立后调用
	virtual int onparser(char *, int)			{return -1;}    // 需解析数据包时调用
	virtual int	ontimeout(int Timerid)	{return 0;};	

public:	
	// 发送数据	
	int		    send_(const char *buf, int nLen);
    //packet
    virtual int onpackcomplete(inpack1 *)=0;

    bool        writable();

	// 获取及设置server对象指针
	tcpserver * server(void){return m_pServer;}
	virtual void server(tcpserver *p){m_pServer = p;}
protected:
	int		        m_sock_fd;
    uint32          fdindex_;             
	uint32	        m_SocketType;		    //  
    bool            m_bNeedDel;             // 是否需要delete
    bool            m_bfull;                // 缓冲区满是否已爆
	TimerEvent      m_TcpTimer;
	tcpserver *     m_pServer;
	char	        m_pRecvBuffer[RECV_BUFFER_SIZE];	
	CLoopBuffer*    m_pSendLoopBuffer;
	char	        m_pTmpSendBuffer[SEND_BUFFER_SIZE];
};

#endif  


