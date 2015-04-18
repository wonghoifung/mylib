#ifndef EVENTLOOP_HEADER
#define EVENTLOOP_HEADER

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <map>
#include <boost/noncopyable.hpp>
#include "pack1.hpp"
#include "packparser1.h"
#include "connection.h"
#include "timerheap.hpp"

class tcpserver;

class event_loop : boost::noncopyable
{
public:
    event_loop();
    ~event_loop();
    void init();
    void run();

    bool addlistenfd(int fd, tcpserver* tcpsvr);
    void dellistenfd(int fd);
    bool islistenfd(int fd);

    connection* getconnection(int fd);
    bool addconnection(connection* conn);
    void delconnection(connection* conn);
    int handleaccept(int listenfd);
    int handlesignal(int fd);

    void setwrite(connection* conn);
    void setread(connection* conn);
    
    timerheap& gettimerheap() {return timerheap_;}
    
private:
    bool stop_;
    int epollfd_;
    connection** fdconns_;
    int fdcount_;
    uint32_t fdindex_; 
    epoll_event* epevents_;
    std::map<int,tcpserver*> listenfds_;
    timerheap timerheap_;
};

inline void setnonblock(int fd)
{
    int flags = ::fcntl(fd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    ::fcntl(fd, F_SETFL, flags);
}

inline void setcloseonexec(int fd)
{
    int flags = ::fcntl(fd, F_GETFD, 0);
    flags |= FD_CLOEXEC;
    ::fcntl(fd, F_SETFD, flags);
}

#endif


