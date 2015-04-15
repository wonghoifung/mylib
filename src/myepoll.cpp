#include "myepoll.h"
#include "socketops.h"
#include "common.h"
#include "connection.h"

#include <assert.h>
#include <errno.h>
//#include <poll.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/resource.h>

#define MAX_DESCRIPTORS 100000

myepoll::myepoll() : events_(16)
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

