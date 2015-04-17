#include "eventloop.h"
#include <sys/time.h>
#include <sys/resource.h>

#define MAX_EVENT_COUNT 100000
#define MAX_FD_COUNT 100000

namespace 
{	
    int maxfdcount()
    {
        struct rlimit rl;
        int nfiles = MAX_FD_COUNT;
        if (::getrlimit(RLIMIT_NOFILE, &rl) == 0 && rl.rlim_cur != RLIM_INFINITY) {
            nfiles = rl.rlim_cur - 1;
        }
        if( nfiles > MAX_FD_COUNT ) nfiles = MAX_FD_COUNT;
        return nfiles;
    }
}

event_loop::event_loop() 
:stop_(false), 
 epollfd_(-1),
 fdconns_(NULL),
 fdcount_(0),
 fdindex_(0),
 epevents_(NULL)
{
    init();
}

event_loop::~event_loop()
{
    ::close(epollfd_);
    free(epevents_);
    free(fdconns_);
}

void event_loop::init()
{
    fdconns_ = (connection**)malloc(MAX_FD_COUNT * sizeof(void*));
    memset(fdconns_,0,MAX_FD_COUNT*sizeof(void*));
    if (fdconns_==NULL) { abort(); }

    epollfd_ = ::epoll_create(maxfdcount());
    if (epollfd_==-1) { abort(); }

    epevents_ = (struct epoll_event*)malloc(MAX_EVENT_COUNT*sizeof(epoll_event));
    if (epevents_==NULL) { abort(); }
}

void event_loop::run()
{
    while (!stop_)
    {
        int number = ::epoll_wait( epollfd_, epevents_, MAX_EVENT_COUNT, 100 ); // TODO timer
        if ( number < 0 )
        {
            if (EINTR==errno) { continue; }
            break;
        }
        else if (number == 0)
        {
            // timeout
        }
        else 
        {

        }

        for ( int i = 0; i < number; i++ )
        {
            int sockfd = epevents_[i].data.fd;

            if (islistenfd(sockfd))
            {
                handleaccept(sockfd);
                continue;
            }
			
            uint32_t index = (uint32_t)(epevents_[i].data.u64 >> 32);
            connection* conn = fdconns_[sockfd];
            if (conn==0 || conn->getfdindex()!=index) { continue; }
			
            if (epevents_[i].events & (EPOLLHUP | EPOLLERR))
            {
                delconnection(conn);
                continue;
            }
            else if(epevents_[i].events & EPOLLIN)
            {				
                if( conn->onread() == -1 )
                {
                    delconnection(conn);
                    continue;
                }
                if( conn->needtowritesocket() )
                    setwrite(conn);
            }
            else if(epevents_[i].events & EPOLLOUT)
            {
                if( conn->onwrite() == -1 )
                {
                    delconnection(conn);
                    continue;
                }
                if( !conn->needtowritesocket() )
                    setread(conn);
            }
        }
    }
}

bool event_loop::addlistenfd(int fd)
{
    if (listenfds_.insert(fd).second)
    {
        struct epoll_event ev;
        ev.data.fd = fd;
        ev.events = EPOLLIN | EPOLLET; // ET mode for listen fd
        ::epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev);
        return true;
    }
    return false;
}

void event_loop::dellistenfd(int fd)
{
    struct epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));
    ev.data.fd = fd;
    ev.events =  EPOLLIN | EPOLLET;
    ::epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, &ev);
    listenfds_.erase(fd);
}

bool event_loop::islistenfd(int fd)
{
    return (listenfds_.find(fd) != listenfds_.end());
}

connection* event_loop::getconnection(int fd)
{
    return fdconns_[fd];
}

bool event_loop::addconnection(connection* conn)
{
    if (conn==NULL) { return false; }
    ++fdcount_;
    ++fdindex_;
    conn->setfdindex(fdindex_);
    if (conn->getfd() >= MAX_FD_COUNT) {abort();}
    if (fdconns_[conn->getfd()] != 0) {abort();}
    fdconns_[conn->getfd()] = conn;

    epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));
    // store the generation counter in the upper 32 bits, the fd in the lower 32 bits 
    ev.data.u64 = (uint64_t)(uint32_t)(conn->getfd()) | ((uint64_t)(uint32_t)(conn->getfdindex()) << 32);
    ev.events = EPOLLIN;
    ::epoll_ctl(epollfd_, EPOLL_CTL_ADD, conn->getfd(), &ev);

    conn->onconnected();
    return true;
}

int event_loop::handleaccept(int listenfd)
{
    int connfd;
    do { // ET listen fd

        struct sockaddr_in client_address;
        socklen_t client_addrlength = sizeof client_address;
        connfd = ::accept( listenfd, ( struct sockaddr* )&client_address, &client_addrlength );
        if (connfd < 0) { break; } // no new connection 
		
        int opt = 16*1024;
        socklen_t optlen = sizeof opt;
        ::setsockopt(connfd, SOL_SOCKET, SO_RCVBUF, &opt, optlen);
        ::setsockopt(connfd, SOL_SOCKET, SO_SNDBUF, &opt, optlen);

        int opts = fcntl(connfd, F_GETFL);
        if(opts < 0) { continue; } // TODO log
        opts = opts | O_NONBLOCK;
        if(fcntl(connfd, F_SETFL, opts) < 0) { continue; }

        connection* conn = new connection(connfd,this);
        if (conn==NULL) {
            ::close(connfd);
            continue;
        }

        addconnection(conn);

    } while(connfd > 0);
    return 0;
}

void event_loop::delconnection(connection* conn)
{
    assert(conn!=NULL);
    conn->onclosed();

    --fdcount_;
    assert(fdconns_[conn->getfd()]==conn);
    fdconns_[conn->getfd()] = 0;
    struct epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));
    ev.data.u64 = (uint64_t)(uint32_t)(conn->getfd()) | ((uint64_t)(uint32_t)(conn->getfdindex()) << 32);
    ev.events =  EPOLLOUT | EPOLLIN;
    ::epoll_ctl(epollfd_, EPOLL_CTL_DEL, conn->getfd(), &ev);
    ::close(conn->getfd());

    // TODO reuse
    delete conn;
    conn = NULL;
}

void event_loop::setwrite(connection* conn)
{
    struct epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));
    ev.data.u64 = (uint64_t)(uint32_t)(conn->getfd()) | ((uint64_t)(uint32_t)(conn->getfdindex()) << 32);
    ev.events = EPOLLOUT ;
    ::epoll_ctl(epollfd_, EPOLL_CTL_MOD, conn->getfd(), &ev);
}

void event_loop::setread(connection* conn)
{
    struct epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));
    ev.data.u64 = (uint64_t)(uint32_t)(conn->getfd()) | ((uint64_t)(uint32_t)(conn->getfdindex()) << 32);
    ev.events = EPOLLIN ;
    ::epoll_ctl(epollfd_, EPOLL_CTL_MOD, conn->getfd(), &ev);
}


