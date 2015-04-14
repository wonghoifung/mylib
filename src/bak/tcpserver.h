#ifndef TCPSERVER_HEADER
#define TCPSERVER_HEADER

#include <sys/resource.h>
#include <sys/times.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <map>
#include <vector>
#include "tcphandler.h"
#include "common.h"

class tcpserver
{
public:
	tcpserver();
	virtual ~tcpserver();
    static void sighandler(int signum);
	bool initsock(int listen_port);	
	bool run();
	virtual tcphandler* createhandler(void) = 0;
	tcphandler* allocatehandler(SOCKET sock_fd);
	bool reg(tcphandler* pHandler);

	// close connection on one's own initiative
	bool disconnect(tcphandler* pSocketHandler); 

    void want_to_write(tcphandler* s);
    void want_to_read(tcphandler* s);

protected:
	int handle_accept();
    void addsocket(tcphandler* s);
    void delsocket(tcphandler* s);
	bool init_event();
    void handle_close(tcphandler* pHandler);

protected:
    static bool run_;
    SOCKET listenfd_;
	int maxfd_;
    tcphandler** handles_;    
    int countfd_;
    uint32 fdindex_;
	int epollfd_;
	struct epoll_event* epollevarr_;	
};

#endif

