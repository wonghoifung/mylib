#include "TcpHandler.h"
#include "log.h"
#include "SockerAPI.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "TcpServer.h"

#ifdef WIN32
    #ifndef EWOULDBLOCK
        #define EWOULDBLOCK             WSAEWOULDBLOCK
    # endif /* !EWOULDBLOCK */
#else
	#include <sys/types.h>
	#include <sys/socket.h>
#endif // WIN32

TcpHandler::TcpHandler()
:TimerOutEvent()
,m_sock_fd(0)
,m_fd_index(0)
,m_bfull(false)
{
    m_bNeedDel = false;
	memset(m_pRecvBuffer,0,sizeof(m_pRecvBuffer));
	m_pServer = NULL;
    m_TcpTimer.SetTimeEventObj(this);

    m_pSendLoopBuffer = new CLoopBuffer(MAX_LOOP_BUFFER_LEN);
}
TcpHandler::~TcpHandler()
{
	if(m_pSendLoopBuffer)
		delete m_pSendLoopBuffer;
	m_pSendLoopBuffer = NULL;
}

void TcpHandler::SetFd(int sock_fd)
{
	m_sock_fd = sock_fd;
}
int TcpHandler::GetFd()const
{
	return m_sock_fd;
}

int TcpHandler::handle_OnConnected()
{
	return OnConnected();
}
int TcpHandler::handle_read()
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
        int ret = OnParser(m_pRecvBuffer, nRecv);
        if(ret != 0)
            return -1;
        if(nRecv < buff_size)
            return 0;
    } 
    return -1;
}

int TcpHandler::handle_output()
{
    if(!Writable())
        return 0;
    if(m_bfull)
        return -1;

    int nPeekLen = 0;
    int nHaveSendLen = 0;
    do 
    {
        nPeekLen = m_pSendLoopBuffer->Peek(m_pTmpSendBuffer,sizeof(m_pTmpSendBuffer));
        nHaveSendLen = CSocker::SocketSend(GetFd(),m_pTmpSendBuffer, nPeekLen);

        //Send data block
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

int TcpHandler::handle_close()
{
	m_TcpTimer.StopTimer();
	OnClose();
	return 0;
}

int TcpHandler::Send(const char *buf, int nLen)
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

    if(Writable())
        m_pServer->WantWrite(this);

	return 0;
}
bool TcpHandler::Writable()
{
    return ( m_pSendLoopBuffer->DataCount()>0 ) ? true : false;
}


