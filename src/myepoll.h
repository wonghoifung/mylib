#ifndef MYEPOLL_HEADER
#define MYEPOLL_HEADER

#include <time.h>
#include <vector>
#include <map>
#include <boost/noncopyable.hpp>

struct epoll_event;
class connection;
class eventloop;

class myepoll : boost::noncopyable
{
public:
	myepoll(eventloop* loop);
	virtual ~myepoll();
	virtual time_t poll(int timeoutms, std::vector<connection*>* activeconns);
	virtual void updateconn(connection* conn);
	virtual void removeconn(connection* conn);

private:
	void update(int operation, connection* conn);

private:
	int epollfd_;
	std::vector<struct epoll_event> events_;

	std::map<int, connection*> conns_;
	eventloop* ownerloop_;
};

#endif


