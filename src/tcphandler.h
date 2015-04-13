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

#define  RECV_BUFFER_SIZE (1024*32)		//8k�Ļ�����
#define  SEND_BUFFER_SIZE (1024*32)		//3k�Ļ�����


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
	// �Զ���Hook ����
	virtual int onclose(void)					{return -1;}	// ���ӶϿ������
	virtual int onconnected(void)				{return 0;}	    // ���ӳɹ����������
	virtual int onparser(char *, int)			{return -1;}    // ��������ݰ�ʱ����
	virtual int	ontimeout(int Timerid)	{return 0;};	

public:	
	// ��������	
	int		    send_(const char *buf, int nLen);
    //packet
    virtual int onpackcomplete(inpack1 *)=0;

    bool        writable();

	// ��ȡ������server����ָ��
	tcpserver * server(void){return m_pServer;}
	virtual void server(tcpserver *p){m_pServer = p;}
protected:
	int		        m_sock_fd;
    uint32          fdindex_;             
	uint32	        m_SocketType;		    //  
    bool            m_bNeedDel;             // �Ƿ���Ҫdelete
    bool            m_bfull;                // ���������Ƿ��ѱ�
	TimerEvent      m_TcpTimer;
	tcpserver *     m_pServer;
	char	        m_pRecvBuffer[RECV_BUFFER_SIZE];	
	CLoopBuffer*    m_pSendLoopBuffer;
	char	        m_pTmpSendBuffer[SEND_BUFFER_SIZE];
};

#endif  


