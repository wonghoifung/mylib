#include "eventloop.h"
#include "tcpserver.h"
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>

#define MAX_EVENT_COUNT 100000
#define MAX_FD_COUNT 100000

namespace 
{
    int signal_pipefd[2];
    
    void signal_handler(int sig)
    {
        int savederrno = errno;
        int msg = sig;
        ::send(signal_pipefd[1], (char*)&msg, 1, 0);
        errno = savederrno;
    }
    
    void addsignal(int sig)
    {
        struct sigaction sa;
        memset(&sa, '\0', sizeof(sa));
        sa.sa_handler = signal_handler;
        sa.sa_flags |= SA_RESTART;
        sigfillset(&sa.sa_mask);
        if (sigaction(sig, &sa, NULL)==-1) {
            printf("sig:%d, errno:%d, strerror:%s\n",sig,errno,strerror(errno));
            abort();
        }
    }
    
    void ignoresignal(int sig)
    {
        struct sigaction act;
        act.sa_handler = SIG_IGN;
        if (sigaction(sig, &act, NULL)==-1) {
            abort();
        }
    }
    
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
    
    void init_global_signal_pipefd(int epollfd)
    {
        if (::socketpair(PF_UNIX, SOCK_STREAM, 0, signal_pipefd)==-1) { abort(); }
        setnonblock(signal_pipefd[1]);
        epoll_event ev;
        ev.data.fd = signal_pipefd[0];
        ev.events = EPOLLIN | EPOLLET;
        ::epoll_ctl(epollfd, EPOLL_CTL_ADD, signal_pipefd[0], &ev);
        setnonblock(signal_pipefd[0]);
    }
    
    void fini_global_signal_pipefd(int epollfd)
    {
        struct epoll_event ev;
        memset(&ev, 0, sizeof(epoll_event));
        ev.data.fd = signal_pipefd[0];
        ev.events =  EPOLLIN | EPOLLET;
        ::epoll_ctl(epollfd, EPOLL_CTL_DEL, signal_pipefd[0], &ev);
        ::close(signal_pipefd[1]);
        ::close(signal_pipefd[0]);
    }
}

event_loop::event_loop() 
:stop_(false), 
 epollfd_(-1),
 fdconns_(NULL),
 fdcount_(0),
 fdindex_(0),
 epevents_(NULL),
 timerheap_(100)
{
    init();
}

event_loop::~event_loop()
{
    fini_global_signal_pipefd(epollfd_);
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
    setcloseonexec(epollfd_);

    epevents_ = (struct epoll_event*)malloc(MAX_EVENT_COUNT*sizeof(epoll_event));
    if (epevents_==NULL) { abort(); }
    
    init_global_signal_pipefd(epollfd_);
    
    addsignal(SIGHUP);
    addsignal(SIGCHLD);
    addsignal(SIGTERM);
    addsignal(SIGINT);
    addsignal(SIGURG);
    addsignal(SIGUSR1);
    addsignal(SIGUSR2);
    ignoresignal(SIGPIPE);
}

void event_loop::run()
{
    int timeout(0);
    time_t current(0);
    while (!stop_)
    {
        timeout = 100;
        current = time(NULL);
        timer* t = timerheap_.top();
        if (t) {
            timeout = (t->expire_ - current) * 1000;
            if (timeout < 0) {
                timeout = 0;
            }
        }
        int number = ::epoll_wait( epollfd_, epevents_, MAX_EVENT_COUNT, timeout );
        if ( number < 0 )
        {
            if (EINTR==errno) { continue; }
            break;
        }
        //else if (number == 0)
        {
            // check timers anyway
            timerheap_.tick();
        }

        for ( int i = 0; i < number; i++ )
        {
            int sockfd = epevents_[i].data.fd;

            if(sockfd==signal_pipefd[0])
            {
                if (epevents_[i].events & EPOLLIN) {
                    if (handlesignal(sockfd)==-1) {
                        // log
                        fini_global_signal_pipefd(epollfd_);
                        init_global_signal_pipefd(epollfd_);
                    }
                } else {
                    // log
                    fini_global_signal_pipefd(epollfd_);
                    init_global_signal_pipefd(epollfd_);
                }
                continue;
            }
            else if (islistenfd(sockfd))
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

bool event_loop::addlistenfd(int fd, tcpserver* tcpsvr)
{
    if (listenfds_.insert(std::make_pair(fd,tcpsvr)).second)
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
    
    // don't reuse, because connection use too much space
    delete conn;
    conn = NULL;
}

int event_loop::handleaccept(int listenfd)
{
    int connfd;
    do { // ET listen fd

        tcpserver* tcpsvr = listenfds_[listenfd];
        
        struct sockaddr_in client_address;
        socklen_t client_addrlength = sizeof client_address;
        connfd = ::accept( listenfd, ( struct sockaddr* )&client_address, &client_addrlength );
        if (connfd < 0) {
            if (errno==EMFILE) {
                // method by Marc Lehmann, author of livev
                ::close(tcpsvr->getidlefd());
                int fd = ::accept(listenfd, NULL, NULL);
                ::close(fd);
                fd = ::open("/dev/null", O_RDONLY);
                setcloseonexec(fd);
                tcpsvr->setidlefd(fd);
                // log
            }
            break; // no new connection
        }
		
        int opt = 16*1024;
        socklen_t optlen = sizeof opt;
        ::setsockopt(connfd, SOL_SOCKET, SO_RCVBUF, &opt, optlen);
        ::setsockopt(connfd, SOL_SOCKET, SO_SNDBUF, &opt, optlen);
        setnonblock(connfd);
        setcloseonexec(connfd);
        
        connection* conn = new connection(connfd,this);
        if (conn==NULL) {
            ::close(connfd);
            continue;
        }
        
        conn->set_inpack1callback(tcpsvr->get_inpack1callback());
        conn->set_connectedcallback(tcpsvr->get_connectedcallback());
        conn->set_closedcallback(tcpsvr->get_closedcallback());
        addconnection(conn);

    } while(connfd > 0);
    return 0;
}

int event_loop::handlesignal(int fd)
{
    char signals[1024] = {0};
    int ret = ::recv(fd, signals, sizeof(signals), 0);
    if (ret > 0) {
        for (int i=0; i<ret; ++i) {
            switch (signals[i]) {
                case SIGCHLD:
                case SIGHUP:
                    return 0;
                case SIGTERM:
                case SIGINT:
                case SIGUSR1:
                case SIGUSR2:
                    stop_ = true;
                    return 0;
                case SIGURG:
                    // log out-of-band data
                    return 0;
                default:
                    // log unknown signal
                    return 0;
            }
        }
    }
    
    if (ret==-1 &&
        (errno==EAGAIN ||
         errno==EWOULDBLOCK ||
         errno==EINTR)) { return 0; }
    
    return -1;
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


