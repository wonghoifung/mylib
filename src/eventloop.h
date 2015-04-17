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
#include <set>
#include <boost/noncopyable.hpp>
#include "pack1.h"
#include "packparser1.h"
#include "connection.h"

class event_loop : boost::noncopyable
{
public:
    event_loop();
    ~event_loop();
    void init();
    void run();

    bool addlistenfd(int fd);
    void dellistenfd(int fd);
    bool islistenfd(int fd);

    bool addconnection(connection* conn);
    int handleaccept(int listenfd);
    void delconnection(connection* conn);

    void setwrite(connection* conn);
    void setread(connection* conn);

private:
    bool stop_;
    int epollfd_;
    connection** fdconns_;
    int fdcount_;
    uint32_t fdindex_; 
    epoll_event* epevents_;
    std::set<int> listenfds_;
};

#endif


