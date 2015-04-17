#ifndef TCPSERVER_HEADER
#define TCPSERVER_HEADER

#include <boost/noncopyable.hpp>

class event_loop;

class tcpserver : boost::noncopyable
{
public:
	explicit tcpserver(event_loop* eloop);
	~tcpserver();
	int init(int listenport);

private:
	int listenfd_;
	event_loop* evloop_;
};

#endif
