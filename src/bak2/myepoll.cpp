#include "myepoll.h"
#include "socketops.h"
#include "common.h"
#include "connection.h"
#include "eventloop.h"

#include <assert.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/resource.h>

#define NEWCONN -1
#define ADDEDCONN 1
#define DELETEDCONN 2

#define MAX_DESCRIPTORS 100000

myepoll::myepoll(eventloop* loop) : events_(16), ownerloop_(loop)
{
	struct rlimit rl;
	int nfiles = MAX_DESCRIPTORS;
	if (getrlimit(RLIMIT_NOFILE, &rl) == 0 && rl.rlim_cur != RLIM_INFINITY) {
			nfiles = rl.rlim_cur - 1;
	}
	if( nfiles > MAX_DESCRIPTORS ) nfiles = MAX_DESCRIPTORS;

	epollfd_ = ::epoll_create(nfiles); // ::epoll_create1(EPOLL_CLOEXEC)
	socketops::set_cloexec(epollfd_);
	if (epollfd_ < 0)
	{
		printf("cannot create epoll\n");
		abort();
	}
}

myepoll::~myepoll()
{
	::close(epollfd_);
}

time_t myepoll::poll(int timeoutms, std::vector<connection*>* activeconns)
{
	int evnum = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutms);
	int savederrno = errno;
	time_t now = time(NULL);
	if (evnum > 0) {
		assert(implicit_cast<unsigned>(evnum) <= events_.size());
		for (int i=0; i<evnum; ++i) {
			connection* conn = static_cast<connection*>(events_[i].data.ptr);
			conn->set_revents(events_[i].events);
			activeconns->push_back(conn);
		}
		if (implicit_cast<unsigned>(evnum) == events_.size()) {
			events_.resize(events_.size()*2);
		}
	}
	else if (evnum == 0) {
		printf("nothing happened");
	}
	else {
		if (savederrno != EINTR) {
			errno = savederrno;
			printf("error happens when epoll_wait\n");
		}
	}
	return now;
}

void myepoll::updateconn(connection* conn)
{
	const int index = conn->index();
	if (index == NEWCONN || index == DELETEDCONN)
	{
		int fd = conn->fd();
		if (index == NEWCONN)
		{
			assert(conns_.find(fd) == conns_.end());
			conns_[fd] = conn;
		}
		else 
		{
			assert(conns_.find(fd) != conns_.end());
			assert(conns_[fd] == conn);
		}
		conn->set_index(ADDEDCONN);
		update(EPOLL_CTL_ADD, conn);
	}
	else
	{
		int fd = conn->fd();
		(void)fd;
		assert(conns_.find(fd) != conns_.end());
		assert(conns_[fd] == conn);
		assert(index == ADDEDCONN);
		if (conn->isnoneevent())
		{
			update(EPOLL_CTL_DEL, conn);
			conn->set_index(DELETEDCONN);
		}
		else
		{
			update(EPOLL_CTL_MOD, conn);
		}
	}
}

void myepoll::removeconn(connection* conn)
{
	int fd = conn->fd();
	assert(conns_.find(fd) != conns_.end());
	assert(conns_[fd] == conn);
	assert(conn->isnoneevent());
	int index = conn->index();
	assert(index == ADDEDCONN || index == DELETEDCONN);
	size_t n = conns_.erase(fd);
	assert(n == 1);
	if (index == ADDEDCONN)
	{
		update(EPOLL_CTL_DEL, conn);
	}
	conn->set_index(NEWCONN);
}

void myepoll::update(int operation, connection* conn)
{
	struct epoll_event event;
	bzero(&event, sizeof event);
	event.events = conn->events();
	event.data.ptr = conn;
	int fd = conn->fd();
	if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
	{
		if (operation == EPOLL_CTL_DEL)
		{
			printf("epoll_ctl cannot del fd:%d\n",fd);
		}
		else
		{
			printf("epoll_ctl cannot op:%d fd:%d\n",operation,fd);
			abort();
		}
	}
}

