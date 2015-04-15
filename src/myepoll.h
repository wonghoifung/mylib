#ifndef MYEPOLL_HEADER
#define MYEPOLL_HEADER

#include <time.h>
#include <vector>
#include <boost/noncopyable.hpp>

struct epoll_event;
class connection;

class myepoll : boost::noncopyable
{
public:
	myepoll();
	virtual ~myepoll();
	virtual time_t poll(int timeoutms, std::vector<connection*>* activeconns);

private:
	int epollfd_;
	std::vector<struct epoll_event> events_;
};

#endif


