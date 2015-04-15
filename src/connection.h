#ifndef CONNECTION_HEADER
#define CONNECTION_HEADER

#include <time.h>
#include <sys/epoll.h>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

#include "packparser1.h"
#include "pack1.h"

#define NONEEVENT 0
#define READEVENT (EPOLLIN | EPOLLPRI)
#define WRITEEVENT (EPOLLOUT)

#define RECV_BUFFER_SIZE (1024*32)	
#define SEND_BUFFER_SIZE (1024*32)	

class eventloop;

typedef boost::function<void()> EventCallback;
typedef boost::function<void(time_t)> ReadEventCallback;
typedef boost::function<void(inpack1*)> inpack1Callback;

class connection : boost::noncopyable, public ipackparser
{
public:
	connection(eventloop* loop, int fd);
	~connection();

	void set_islistenfd(bool b) { islistenfd_ = b; }
	bool get_islistenfd() { return islistenfd_; }

	void setreadcallback(const ReadEventCallback& cb) { readcallback_ = cb; }
	void setwritecallback(const EventCallback& cb) { writecallback_ = cb; }
	void setclosecallback(const EventCallback& cb) { closecallback_ = cb; }
	void seterrorcallback(const EventCallback& cb) { errorcallback_ = cb; }
	void setinpack1callback(const inpack1Callback& cb) { inpack1callback_ = cb; }

	int fd() const { return fd_; }
	int events() const { return events_; }
	void set_revents(int revt) { revents_ = revt; }

	int index() { return index_; }
	void set_index(int idx) { index_ = idx; }

	bool isnoneevent() const { return events_ == NONEEVENT; }
	void enablereading() { events_ |= READEVENT; update(); }
	void enablewriting() { events_ |= WRITEEVENT; update(); }
	void disablewriting() { events_ &= ~WRITEEVENT; update(); }
	void disableall() { events_ = NONEEVENT; update(); }
	bool iswriting() const { return events_ & WRITEEVENT; }
	void remove();

	void handle_event(time_t receivetime);
	int handle_read(time_t receivetime);

	virtual int parsepack(const char* , const size_t);
	virtual int onpackcomplete(inpack1*);

private:
	void update();

private:
	eventloop* loop_;
	const int fd_;
	int events_;
	int revents_; 
	int index_; 

	bool addedtoloop_;
	ReadEventCallback readcallback_;
	EventCallback writecallback_;
	EventCallback closecallback_;
	EventCallback errorcallback_;
	inpack1Callback inpack1callback_;

	bool islistenfd_;
	bool bfull_;
	ipackparser* parser_;
	char recvbuf_[RECV_BUFFER_SIZE];
	char sendbuf_[SEND_BUFFER_SIZE];
};
typedef boost::shared_ptr<connection> connectionptr;

#endif
