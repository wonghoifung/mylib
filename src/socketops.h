#ifndef SOCKETOPS_HEADER
#define SOCKETOPS_HEADER

#include <arpa/inet.h>

namespace socketops
{
	int mysocket_nonblock();
	int myconnect(int sockfd, const struct sockaddr_in& addr);
	void mybind(int sockfd, const struct sockaddr_in& addr);
	void mylisten(int sockfd);
	int myaccept(int sockfd, struct sockaddr_in* addr);
	int myread(int sockfd, void *buf, size_t count);
	int myreadv(int sockfd, const struct iovec *iov, int iovcnt);
	int mywrite(int sockfd, const void *buf, size_t count);
	void myclose(int sockfd);
	void myshutdownwrite(int sockfd);
	void toipport(char* buf, size_t size, const struct sockaddr_in& addr);
	void toip(char* buf, size_t size, const struct sockaddr_in& addr);
	void fromipport(const char* ip, uint16_t port, struct sockaddr_in* addr);
	int getsocketerror(int sockfd);
	const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
	struct sockaddr* sockaddr_cast(struct sockaddr_in* addr);
	const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr);
	struct sockaddr_in* sockaddr_in_cast(struct sockaddr* addr);
	struct sockaddr_in getlocaladdr(int sockfd);
	struct sockaddr_in getpeeraddr(int sockfd);
	bool isselfconnect(int sockfd);
}

#endif
