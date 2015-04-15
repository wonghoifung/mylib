#include "connection.h"

#include <sys/epoll.h>

connection::connection(eventloop* loop, int fd)
:loop_(loop),
 fd_(fd),
 events_(0),
 revents_(0)
{

}

connection::~connection()
{

}

void connection::handle_event(time_t receivetime)
{
	if ((revents_&EPOLLHUP) && !(revents_&EPOLLIN))
	{
		// TODO close
	}

	if (revents_&EPOLLERR)
	{
		// TODO error
	}

	//if (revents_&(EPOLLIN|EPOLLPRI|EPOLLRDHUP))
	if (revents_&(EPOLLIN|EPOLLPRI))
	{
		// TODO read
	}

	if (revents_&EPOLLOUT)
	{
		// TODO write
	}
}
