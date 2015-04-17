#include "tcpserver.h"
#include "eventloop.h"

tcpserver::tcpserver(event_loop* eloop):listenfd_(-1),evloop_(eloop)
{

}

tcpserver::~tcpserver()
{
	if (listenfd_!=-1) {
		evloop_->dellistenfd(listenfd_);
		::close(listenfd_);
	}
}

int tcpserver::init(int listenport)
{
	listenfd_ = ::socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd_ == -1) abort();

	int opt = 1;
	::setsockopt(listenfd_, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));	

	int opts = fcntl(listenfd_, F_GETFL);
	if( opts < 0 ) abort();
	opts = opts | O_NONBLOCK;
	if( fcntl(listenfd_, F_SETFL, opts) < 0) abort();

	struct sockaddr_in addr;
	memset( &addr , 0,sizeof(addr) );
	addr.sin_family	= AF_INET;
	addr.sin_port = htons(listenport);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if ( 0 != bind( listenfd_ , (struct sockaddr *)&addr , sizeof(addr) ) ) abort();
	if( 0 != listen( listenfd_, 1000000 ) ) abort();

	if (!evloop_->addlistenfd(listenfd_)) abort();
	return 0;
}


