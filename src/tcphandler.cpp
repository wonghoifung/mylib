#include "tcphandler.h"
#include "log.h"
#include "socketops.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "tcpserver.h"

#ifdef WIN32
    #ifndef EWOULDBLOCK
        #define EWOULDBLOCK             WSAEWOULDBLOCK
    # endif /* !EWOULDBLOCK */
#else
	#include <sys/types.h>
	#include <sys/socket.h>
#endif // WIN32

tcphandler::tcphandler()
:TimerOutEvent()
,m_sock_fd(0)
,fdindex_(0)
,m_bfull(false)
{
    m_bNeedDel = false;
	memset(m_pRecvBuffer,0,sizeof(m_pRecvBuffer));
	m_pServer = NULL;
    m_TcpTimer.SetTimeEventObj(this);

    m_pSendLoopBuffer = new CLoopBuffer(MAX_LOOP_BUFFER_LEN);
}
tcphandler::~tcphandler()
{
	if(m_pSendLoopBuffer)
		delete m_pSendLoopBuffer;
	m_pSendLoopBuffer = NULL;
}

void tcphandler::setfd(int sock_fd)
{
	m_sock_fd = sock_fd;
}
int tcphandler::getfd()const
{
	return m_sock_fd;
}

int tcphandler::handle_connected()
{
	return onconnected();
}
int tcphandler::handle_read()
{
    if(m_bfull)
        return -1;

    const int buff_size = sizeof(m_pRecvBuffer);
    while(1) 
    {
        int nRecv = recv(m_sock_fd,m_pRecvBuffer,buff_size,0);
        if(nRecv < 0)
        {
            if(EAGAIN == errno || EWOULDBLOCK == errno)
            {
                //此时连接可用，只是读不到数据，应该continue                			
                return 0;
            }
            return -1;
        }
        if(nRecv == 0)/* 无法感知网络断开，需要配置心跳包或KEEPALIVE，建议用前者 */
        {
            return -1;/* 忘记close会导致 LT 模式下 CPU 100% */ 
        }
        int ret = onparser(m_pRecvBuffer, nRecv);
        if(ret != 0)
            return -1;
        if(nRecv < buff_size)
            return 0;
    } 
    return -1;
}

int tcphandler::handle_output()
{
    if(!writable())
        return 0;
    if(m_bfull)
        return -1;

    int nPeekLen = 0;
    int nHaveSendLen = 0;
    do 
    {
        nPeekLen = m_pSendLoopBuffer->Peek(m_pTmpSendBuffer,sizeof(m_pTmpSendBuffer));
        nHaveSendLen = socketops::mysend(getfd(),m_pTmpSendBuffer, nPeekLen);

        //send_ data block
        if( nHaveSendLen < 0 ) 
        {
            if(errno != EWOULDBLOCK && errno != EINTR)
            {
                m_pSendLoopBuffer->Erase(nPeekLen);            
                return -1;
            }
            return 0;
        }
        else
        {
            m_pSendLoopBuffer->Erase(nHaveSendLen);
        }
     }while (nHaveSendLen>0 && m_pSendLoopBuffer->DataCount()>0);

   return 0;
}

int tcphandler::handle_close()
{
	m_TcpTimer.StopTimer();
	onclose();
	return 0;
}

int tcphandler::send_(const char *buf, int nLen)
{
    if( nLen > (int)m_pSendLoopBuffer->FreeCount())
    {
        log_debug("SendLoopBuff 已经撑爆 ！！！！\n");
        m_bfull = true;
        return -1;
    }
    else        
	    m_pSendLoopBuffer->Put((char *)buf, nLen);
    handle_output();   

    if(writable())
        m_pServer->want_to_write(this);

	return 0;
}
bool tcphandler::writable()
{
    return ( m_pSendLoopBuffer->DataCount()>0 ) ? true : false;
}


