#include "connection.h"
#include "eventloop.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

connection::connection(int fd, event_loop* eloop):
fd_(fd),
fdindex_(0),
parser1_(ipackparser::createparser(this)),
evloop_(eloop),
full_(false),
sndloopbuf_(new loopbuf(LOOPBUF_SIZE))
{
    if (parser1_==NULL) {
        abort();
    }
    if (sndloopbuf_==NULL) { 
        abort();
    }
    memset(recvbuf_,0,sizeof recvbuf_);
    memset(sendbuf_,0,sizeof sendbuf_);
}

connection::~connection()
{
    if (parser1_) {
        delete parser1_;
        parser1_ = NULL;
    }

    if (sndloopbuf_) {
        delete sndloopbuf_;
        sndloopbuf_ = NULL;
    }	
}

int connection::onpackcomplete(inpack1* pack)
{
    printf("cmd: %d\n", pack->getcmd());
    printf("int: %d\n", pack->readint());
    printf("str: %s\n", pack->readstring().c_str());
    return 0;
}

int connection::send(outpack1* pack)
{
    return send(pack->packet_buf(),pack->packet_size());
}

int connection::send(const char* buf, int len)
{
    if ((unsigned long)len > sndloopbuf_->freecount()) {
        full_ = true;
        return -1;
    }
    else {
        sndloopbuf_->put((char*)buf,(unsigned long)len);
    }

    socketwrite();

    if (needtowritesocket()) {
        evloop_->setwrite(this);
    }
    return 0;
}

int connection::socketwrite()
{
    if (!needtowritesocket()) {
        return 0;
    }
    // TODO
    //if(full_) return -1;
    int peeklen = 0;
    int havesentlen = 0;
    do {
        peeklen = sndloopbuf_->peek(sendbuf_,sizeof sendbuf_);
        havesentlen = ::send( fd_, sendbuf_, peeklen, 0 );
        if (havesentlen<0) {
            if(errno != EWOULDBLOCK && errno != EINTR) {
                sndloopbuf_->erase(peeklen); // TODO
                return -1;
            }
            return 0;
        }
        else {
            sndloopbuf_->erase(havesentlen);
        }
    } while (havesentlen > 0 && needtowritesocket());
    return 0;
}

bool connection::needtowritesocket()
{
    return (sndloopbuf_->datacount() > 0);
}

int connection::onconnected()
{
    // TODO
    return 0;
}

int connection::onclosed()
{
    // TODO
    return 0;
}

int connection::onread()
{
    // TODO
    //if(full_) return -1;
	
    const unsigned buff_size = sizeof recvbuf_;
    while(1) {
        unsigned nRecv = recv(fd_, recvbuf_, buff_size, 0);
        if(nRecv < 0) {
            if(EAGAIN == errno || EWOULDBLOCK == errno) { return 0;}
            return -1;
        }
        if(nRecv == 0) return -1; 
        int ret = parser1_->parsepack(recvbuf_, nRecv);
        if(ret != 0) return -1;
        if(nRecv < buff_size) return 0;
    } 
    return -1;
}

int connection::onwrite()
{
    return socketwrite();
}


