#ifndef CONNECTION_HEADER
#define CONNECTION_HEADER

#include <time.h>
#include <boost/noncopyable.hpp>

class eventloop;

class connection : boost::noncopyable
{
public:
	connection(eventloop* loop, int fd);
	~connection();

	int fd() const { return fd_; }
	int events() const { return events_; }
	void set_revents(int revt) { revents_ = revt; }
	void handle_event(time_t receivetime);

private:
	eventloop* loop_;
	const int fd_;
	int events_;
	int revents_; 
};

#endif
