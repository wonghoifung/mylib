#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "tcphandler.h"
#include "log.h"
#include "socketops.h"
#include "tcpserver.h"

tcphandler::tcphandler()
:timerhandler()
,sockfd_(0)
,fdindex_(0)
,full_(false)
{
    willdel_ = false;
	memset(recvbuf_,0,sizeof(recvbuf_));
	tcpserver_ = NULL;
    tcptimer_.settimerhandler(this);
    sendloopbuf_ = new loopbuf(LOOP_BUFFER_SIZE);
}

tcphandler::~tcphandler()
{
	if(sendloopbuf_)
		delete sendloopbuf_;
	sendloopbuf_ = NULL;
}

void tcphandler::setfd(int sock_fd)
{
	sockfd_ = sock_fd;
}

int tcphandler::getfd() const
{
	return sockfd_;
}

int tcphandler::handle_connected()
{
	return onconnected();
}

int tcphandler::handle_read()
{
    if(full_)
        return -1;

    const int buff_size = sizeof(recvbuf_);
    while(1) 
    {
        int nRecv = recv(sockfd_,recvbuf_,buff_size,0);
        if(nRecv < 0)
        {
            if(EAGAIN == errno || EWOULDBLOCK == errno)
            {                			
                return 0;
            }
            return -1;
        }
        if(nRecv == 0)
        {
            return -1;
        }
        int ret = onparser(recvbuf_, nRecv);
        if(ret != 0)
            return -1;
        if(nRecv < buff_size)
            return 0;
    } 
    return -1;
}

int tcphandler::handle_write()
{
    if(!writable())
        return 0;
    if(full_)
        return -1;

    int nPeekLen = 0;
    int nHaveSendLen = 0;
    do 
    {
        nPeekLen = sendloopbuf_->peek(sendbuf_,sizeof(sendbuf_));
        nHaveSendLen = socketops::mysend(getfd(),sendbuf_, nPeekLen);

        //sendpack data block
        if( nHaveSendLen < 0 ) 
        {
            if(errno != EWOULDBLOCK && errno != EINTR)
            {
                sendloopbuf_->erase(nPeekLen);            
                return -1;
            }
            return 0;
        }
        else
        {
            sendloopbuf_->erase(nHaveSendLen);
        }
     }while (nHaveSendLen>0 && sendloopbuf_->datacount()>0);

   return 0;
}

int tcphandler::handle_close()
{
	tcptimer_.stoptimer();
	onclose();
	return 0;
}

int tcphandler::sendpack(const char* buf, int nLen)
{
    if(nLen > (int)sendloopbuf_->freecount())
    {
        log_debug("sendloopbuf_ is full");
        full_ = true;
        return -1;
    }
    else        
	    sendloopbuf_->put((char *)buf, nLen);
    handle_write();   

    if(writable())
        tcpserver_->want_to_write(this);

	return 0;
}
bool tcphandler::writable()
{
    return ( sendloopbuf_->datacount()>0 ) ? true : false;
}


