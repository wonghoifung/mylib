#include "acceptor.h"
#include "socketops.h"

#include <errno.h>
#include <fcntl.h>
#include <boost/bind.hpp>

acceptor::acceptor(eventloop* loop, const netaddr& listenaddr, const std::string& name, bool reuseport)
:loop_(loop),
 acceptsockfd_(socketops::mysocket_nonblock()),
 acceptconn_(loop,acceptsockfd_),
 listenning_(false),
 idlefd_(::open("/dev/null",O_RDONLY/* | O_CLOEXEC*/)),
 hostport_(listenaddr.topipport()),
 name_(name),
 nextconnid_(1)
{
	assert(idlefd_>=0);
	socketops::set_cloexec(idlefd_); // not thread safe, but compatible with old kernel
	socketops::set_reuseaddr(acceptsockfd_, true);
	socketops::set_reuseport(acceptsockfd_, reuseport);
	socketops::mybind(acceptsockfd_, listenaddr.getsockaddrin());
	acceptconn_.set_islistenfd(true);
	acceptconn_.setreadcallback(boost::bind(&acceptor::handle_accept,this));
}

acceptor::~acceptor()
{
	acceptconn_.disableall();
	acceptconn_.remove();
	::close(idlefd_);
}

void acceptor::listen()
{
	assert(!listenning_);
	listenning_ = true;
	socketops::mylisten(acceptsockfd_);
	acceptconn_.enablereading();
}

std::string acceptor::new_conn_name()
{
	char buf[32] = {0};
	snprintf(buf, sizeof buf, ":%s#%d", hostport_.c_str(), nextconnid_);
	++nextconnid_;
	std::string connName = name_ + buf;
	return connName;
}

void acceptor::handle_accept()
{
	netaddr peeraddr;
	struct sockaddr_in addr;
	bzero(&addr, sizeof addr);
	int connfd = socketops::myaccept(acceptsockfd_, &addr);
	if (connfd >= 0)
	{
		peeraddr.setsockaddrin(addr);
		if (1)
		{

			printf("new connection from %s\n",peeraddr.topipport().c_str());
			connectionptr pcon(new connection(loop_,connfd));
			conns_[new_conn_name()] = pcon;
			
// 			pcon->setclosecallback();
// 			pcon->seterrorcallback();
// 			pcon->setreadcallback();
// 			pcon->setwritecallback();
			pcon->setinpack1callback(boost::bind(&acceptor::handle_inpack1,this,_1));
			pcon->enablereading();
			// TODO callback handle_connect
		}
		else
		{
			socketops::myclose(connfd);
		}
	}
	else
	{
		printf("accept error\n");

		/*
		 The special problem of accept()ing when you can't 
		 Many implementations of the POSIX accept function (for example, found in post-2004 Linux) 
		 have the peculiar behaviour of not removing a connection from the pending queue in all error cases. 
		 For example, larger servers often run out of file descriptors (because of resource limits), 
		 causing accept to fail with ENFILE but not rejecting the connection, 
		 leading to libev signalling readiness on the next iteration again (the connection still exists after all), 
		 and typically causing the program to loop at 100% CPU usage. Unfortunately, 
		 the set of errors that cause this issue differs between operating systems,
		 there is usually little the app can do to remedy the situation,
		 and no known thread-safe method of removing the connection to cope with overload is known (to me).
		 One of the easiest ways to handle this situation is to just ignore it when the program encounters an overload,
		 it will just loop until the situation is over. While this is a form of busy waiting, 
		 no OS offers an event-based way to handle this situation, so it's the best one can do. 
		 A better way to handle the situation is to log any errors other than EAGAIN and EWOULDBLOCK, 
		 making sure not to flood the log with such messages, and continue as usual, 
		 which at least gives the user an idea of what could be wrong ("raise the ulimit!"). 
		 For extra points one could stop the ev_io watcher on the listening fd "for a while", 
		 which reduces CPU usage. If your program is single-threaded, 
		 then you could also keep a dummy file descriptor for overload situations (e.g. by opening /dev/null),
		 and when you run into ENFILE or EMFILE, close it, run accept, close that fd, and create a new dummy fd. 
		 This will gracefully refuse clients under typical overload conditions. 
		 The last way to handle it is to simply log the error and exit, as is often done with malloc failures,
		 but this results in an easy opportunity for a DoS attack.

		 By Marc Lehmann, author of livev.
		*/
		if (errno==EMFILE)
		{
			::close(idlefd_);
			idlefd_ = ::accept(acceptsockfd_, NULL, NULL);
			::close(idlefd_);
			idlefd_ = ::open("/dev/null", O_RDONLY /*| O_CLOEXEC*/);
			socketops::set_cloexec(idlefd_);
		}
	}
}

void acceptor::handle_read(time_t receivetime)
{

}

void acceptor::handle_write()
{

}

void acceptor::handle_close()
{

}

void acceptor::handle_error()
{

}

void acceptor::handle_inpack1(inpack1* ipack)
{

}


