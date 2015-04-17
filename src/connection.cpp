#include "connection.h"
#include "eventloop.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

namespace 
{
    std::string strnetaddr(const struct sockaddr_in& addr)
    {
        char buf[64] = {0};
        unsigned size = sizeof buf;
        assert(size >= INET_ADDRSTRLEN);
        ::inet_ntop(AF_INET, &addr.sin_addr, buf, static_cast<socklen_t>(size));
        size_t end = ::strlen(buf);
        uint16_t port = ntohs(addr.sin_port);
        assert(size > end);
        snprintf(buf+end, size-end, ":%u", port);
        return buf;
    }

    struct sockaddr_in localaddr(int sockfd)
    {
        struct sockaddr_in localaddr;
        bzero(&localaddr, sizeof localaddr);
        socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
        if (::getsockname(sockfd, (struct sockaddr*)(&localaddr), &addrlen) < 0)
        {
            // log error
        }
        return localaddr;
    }

    struct sockaddr_in peeraddr(int sockfd)
    {
        struct sockaddr_in peeraddr;
        bzero(&peeraddr, sizeof peeraddr);
        socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
        if (::getpeername(sockfd, (struct sockaddr*)(&peeraddr), &addrlen) < 0)
        {
            // log error
        }
        return peeraddr;
    }
}

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
    if (inpack1cb_) inpack1cb_(this, pack);
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
    if(full_) return -1;
    int peeklen = 0;
    int havesentlen = 0;
    do {
        peeklen = sndloopbuf_->peek(sendbuf_,sizeof sendbuf_);
        havesentlen = ::send( fd_, sendbuf_, peeklen, 0 );
        if (havesentlen<0) {
            if(errno != EWOULDBLOCK && errno != EINTR) {
                sndloopbuf_->erase(peeklen); 
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

std::string connection::get_localaddr()
{
    struct sockaddr_in addr = localaddr(fd_);
    return strnetaddr(addr);
}

std::string connection::get_peeraddr()
{
    struct sockaddr_in addr = peeraddr(fd_);
    return strnetaddr(addr);
}

int connection::onconnected()
{
    if (connectedcb_) connectedcb_(this);
    return 0;
}

int connection::onclosed()
{
    if (closedcb_) closedcb_(this);
    return 0;
}

int connection::onread()
{
    if(full_) return -1;
	
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


