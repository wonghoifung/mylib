#ifndef SOCKETOPS_HEADER
#define SOCKETOPS_HEADER

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>

#include <vector>
#include <string>

#include "log.h"

namespace socketops
{
	int mylisten(int fd, int port);

	int myaccept(int fd);

	int mysend(int fd, const char* buf, size_t len);

	int myrecv(int fd, void* buf, size_t len);

	void myclose(int fd);    

	int myconnect(int fd, const char* ip, int port);

	int set_nonblock(int fd);

	int set_reuse(int fd);

    int set_keepalive(int fd);

	int myinit(void);

    void set_socketbuf(int fd, int iSize);

	int myconnect_nonblock(int fd, const char* ip, int port);

	int wait_connect(int seconds);
}

#endif
