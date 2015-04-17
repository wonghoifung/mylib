#include "tcpclient.h"
#include "connection.h"
#include "eventloop.h"

tcpclient::tcpclient(event_loop* eloop):connfd_(-1),evloop_(eloop)
{
}

tcpclient::~tcpclient()
{
    if (connfd_ != -1) {
        connection* conn = evloop_->getconnection(connfd_);
        if(conn)evloop_->delconnection(conn);
        ::close(connfd_);
    }
}

int tcpclient::connect(const char* ip, int port)
{
    connfd_ = ::socket(PF_INET,SOCK_STREAM,0);
    if(connfd_<0){
        // TODO log
        return -1;
    }   
    struct sockaddr_in srv_addr;
    memset(&srv_addr,0,sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = inet_addr(ip);
    srv_addr.sin_port = htons(port);
    int ret = ::connect(connfd_, (struct sockaddr*)&srv_addr, sizeof srv_addr);
    if(ret==-1){
        ::close(connfd_);
        connfd_ = -1;
        return -1;
    }
    connection* conn = new connection(connfd_,evloop_);
    if (conn==NULL) {
        ::close(connfd_);
        connfd_ = -1;
        return -1;
    }
    evloop_->addconnection(conn);
    return 0;
}

bool tcpclient::send(outpack1* pack)
{
    connection* conn = evloop_->getconnection(connfd_);
    if (conn) {
        return conn->send(pack)==0;
    }
    return false;
}




