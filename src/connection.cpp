#include "connection.h"
#include "eventloop.h"
#include "socketops.h"

#include <assert.h>

connection::connection(eventloop* loop, int fd)
:loop_(loop),
 fd_(fd),
 events_(0),
 revents_(0),
 index_(-1),
 addedtoloop_(false),
 islistenfd_(false),
 bfull_(false),
 parser_(NULL)
{

}

connection::~connection()
{
	assert(!addedtoloop_);
	if (parser_)
	{
		delete parser_;
		parser_ = NULL;
	}
}

void connection::handle_event(time_t receivetime)
{
	if ((revents_&EPOLLHUP) && !(revents_&EPOLLIN))
	{
		if (closecallback_) { closecallback_(); }
	}

	if (revents_&EPOLLERR)
	{
		if (errorcallback_) { errorcallback_(); }
	}

	//if (revents_&(EPOLLIN|EPOLLPRI|EPOLLRDHUP))
	if (revents_&(EPOLLIN|EPOLLPRI))
	{
		if (get_islistenfd()) {
			if(readcallback_)readcallback_(receivetime);
		}
		else {
			handle_read(receivetime);
		}
	}

	if (revents_&EPOLLOUT)
	{
		if (writecallback_) { writecallback_(); }
	}
}

void connection::handle_read(time_t receivetime)
{
	if(bfull_) return -1;
	const unsigned buff_size = sizeof(recvbuf_);
    
    int nRecv = socketops::myread(fd_,recvbuf_,buff_size);
        
    if (nRecv > 0) {
        int ret = parsepack(recvbuf_, nRecv);
        if(ret != 0)
            return -1;
        if(nRecv < (int)buff_size)
            return 0;
    }
    else if (nRecv == 0) {
        handle_close();
    }
    else {
        if(EAGAIN == errno || EWOULDBLOCK == errno) {
            return;
        }
        handle_error();
    }
}

void connection::handle_write()
{
    
}

void connection::handle_close()
{
    
}

void connection::handle_error()
{
    
}

int connection::parsepack(const char* buf, const size_t len)
{
	if(parser_ == NULL)
		parser_ = ipackparser::createparser(this);
	return parser_->parsepack(buf, len);
}

int connection::onpackcomplete(inpack1* pack)
{
	if (inpack1callback_)
	{
		inpack1callback_(pack);
	}
	return 0;
}

void connection::update()
{
	addedtoloop_ = true;
	loop_->updateconn(this);
}

void connection::remove()
{
	assert(isnoneevent());
	addedtoloop_ = false;
	loop_->removeconn(this);
}

