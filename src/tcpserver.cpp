#include "tcpserver.h"
#include "timer.h"
#include "llist.h"
#include "log.h"
#include "socketops.h"
#include <time.h>
#include <assert.h>
#include <signal.h>

#define EVENT_TOTAL_COUNT	100000
#define MAX_DESCRIPTORS     100000

bool tcpserver::run_ = true;

tcpserver::tcpserver()
{
    countfd_ = 0;
    fdindex_ = 0;
    handles_ = NULL;
}

tcpserver::~tcpserver()
{	
    socketops::myclose(listenfd_);
    socketops::myclose(epollfd_);
    free(epollevarr_);
    free(handles_);  
}

void tcpserver::sighandler(int signum)
{
    if(signum == SIGTERM || signum == SIGUSR1 || signum == SIGKILL)
    {
        log_error("recv signal: %d, process done...", signum);
        tcpserver::run_ = false;  
    } 
}

bool tcpserver::initsock(int listen_port)
{
	listenfd_ = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd_ == -1)
		return false;

    socketops::set_reuse(listenfd_);
	socketops::set_nonblock(listenfd_);

    int ret = socketops::mylisten(listenfd_, listen_port);
    if(ret < 0)
        return false;

	if(!init_event())
        return false;

	log_debug("server start running, listen:%d", listen_port);
	return true;	
}

bool tcpserver::run()
{
	int loop_times = 0;
	const int timer_check_point = 10;

	while(run_) 
	{
		int res = epoll_wait(epollfd_, epollevarr_, EVENT_TOTAL_COUNT, 100);
		if (res < 0) {
			if (EINTR == errno)
				continue;
			log_debug("epoll_wait return false, errno:%d, errstr:%s", errno, strerror(errno));
			break;
		}
		else if (0 == res) { //timeout	
			loop_times = 0;
			run_timer();
		}
		else {
			if (++loop_times >= timer_check_point) {
				loop_times = 0;
				run_timer();
			}
		}

		for(int i=0; i<res; i++)
		{
			if(epollevarr_[i].data.fd == listenfd_)
			{
				handle_accept();
				continue;
			}
			int fd = (uint32_t)epollevarr_[i].data.u64;
			uint32 index = (uint32_t)(epollevarr_[i].data.u64 >> 32);
			tcphandler* s = handles_[fd];
			if( s == 0 || s->get_fd_index() != index )
			{                      
				continue; // epoll returned invalid fd 
			}
			if(epollevarr_[i].events & ( EPOLLHUP | EPOLLERR ))
			{    
				handle_close(s);
				continue;
			}
			else if(epollevarr_[i].events & EPOLLIN)
			{				
				if( s->handle_read() == -1 )
				{
					handle_close(s);
					continue;
				}
				if( s->writable() )
					want_to_write(s);
			}
			else if( epollevarr_[i].events & EPOLLOUT )
			{
				if( s->handle_output() == -1 )
				{
					handle_close(s);
					continue;
				}
				if( !s->writable() )
					want_to_read(s);
			}
		}
	}
	return true;
}

bool tcpserver::init_event()
{
    handles_ =  (tcphandler**)malloc(MAX_DESCRIPTORS * sizeof(void*));
    memset(handles_, 0, MAX_DESCRIPTORS * sizeof(void*));

	struct rlimit rl;
	int nfiles = MAX_DESCRIPTORS;
	if (getrlimit(RLIMIT_NOFILE, &rl) == 0 &&
		rl.rlim_cur != RLIM_INFINITY) {
			nfiles = rl.rlim_cur - 1;
	}
    if( nfiles > MAX_DESCRIPTORS )
        nfiles = MAX_DESCRIPTORS;

	log_debug("epoll create files:%d", nfiles);
	if (-1 == (epollfd_ = epoll_create(nfiles)))
		return false;
	
    // listen fd use ET mode
	struct epoll_event ev;
	ev.data.fd = listenfd_;
	ev.events = EPOLLIN | EPOLLET;
	epoll_ctl(epollfd_, EPOLL_CTL_ADD, listenfd_, &ev);

	epollevarr_ = (struct epoll_event*)malloc(EVENT_TOTAL_COUNT * sizeof(struct epoll_event));

    signal(SIGTERM, tcpserver::sighandler);    	
    signal(SIGUSR1, tcpserver::sighandler);
	signal(SIGKILL, tcpserver::sighandler);
 
	return true;
}

int tcpserver::handle_accept()
{
	SOCKET conn_fd;
    do 
    {
        if((conn_fd = socketops::myaccept(listenfd_)) == -1)
        {
            break;
        }
        socketops::set_socketbuf(conn_fd,16*1024);
        if(socketops::set_nonblock(conn_fd) < 0)
        {
            log_error("set nonblock failure");
            socketops::myclose(conn_fd);
            assert(false);
            continue;
        }
        if(socketops::set_keepalive(conn_fd) < 0)
        {
            log_error("set keepalive failure");
            socketops::myclose(conn_fd);
            assert(false);
            continue;
        }
        tcphandler* sh = allocatehandler(conn_fd);
        if(sh == NULL)
        {
            log_error("allocate tcphandler failure");
            socketops::myclose(conn_fd);
            assert(false);
            continue;
        }
        addsocket(sh);
        sh->handle_connected();
    } while(conn_fd > 0);

	return 0;
}

void tcpserver::handle_close(tcphandler* pHandler)
{
    assert(pHandler != NULL);
    pHandler->handle_close();

    delsocket(pHandler);

    if(pHandler->getneeddel())
    {
        delete pHandler;
        pHandler = NULL;
    }
}

tcphandler* tcpserver::allocatehandler(SOCKET sock_fd)
{
	tcphandler* sh = createhandler();
	if(sh != NULL)
	{
        sh->setneeddel(true);
		sh->setfd(sock_fd);		
		sh->server(this);
	}
	return sh;
}

bool tcpserver::disconnect(tcphandler* pSocketHandler)
{
    log_debug("disconnect");
    handle_close(pSocketHandler);
 	return true;
}

bool tcpserver::reg(tcphandler* pHandler)
{
    if(pHandler == NULL)
        return false;

    addsocket(pHandler);

    pHandler->server(this);
    pHandler->handle_connected();	

	return true;
}

void tcpserver::addsocket(tcphandler* s)
{
    countfd_++;
    fdindex_++;

    s->set_fd_index(fdindex_);

    assert( s->getfd() < MAX_DESCRIPTORS );
    assert(handles_[s->getfd()] == 0);
    handles_[s->getfd()] = s;

    struct epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));

    ev.data.u64 = (uint64_t)(uint32)(s->getfd()) | ((uint64_t)(uint32)(s->get_fd_index()) << 32);

    ev.events = EPOLLIN;
    epoll_ctl(epollfd_, EPOLL_CTL_ADD, s->getfd(), &ev);
}

void tcpserver::delsocket(tcphandler* s)
{
    countfd_--;

    assert(handles_[s->getfd()] == s);
    handles_[s->getfd()] = 0;

    struct epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));
    ev.data.u64 = (uint64_t)(uint32)(s->getfd()) | ((uint64_t)(uint32)(s->get_fd_index()) << 32);

    ev.events =  EPOLLOUT | EPOLLIN;

    epoll_ctl(epollfd_, EPOLL_CTL_DEL, s->getfd(), &ev);

    socketops::myclose(s->getfd());
}

void tcpserver::want_to_write(tcphandler* s)
{
    struct epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));
    ev.data.u64 = (uint64_t)(uint32)(s->getfd()) | ((uint64_t)(uint32)(s->get_fd_index()) << 32);

    ev.events = EPOLLOUT ;
    epoll_ctl(epollfd_, EPOLL_CTL_MOD, s->getfd(), &ev);
}

void tcpserver::want_to_read(tcphandler* s)
{
    struct epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));
    ev.data.u64 = (uint64_t)(uint32)(s->getfd()) | ((uint64_t)(uint32)(s->get_fd_index()) << 32);

    ev.events = EPOLLIN ;
    epoll_ctl(epollfd_, EPOLL_CTL_MOD, s->getfd(), &ev);
}
