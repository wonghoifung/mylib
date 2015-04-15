#include "socketops.h"
#include "common.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h> 
#include <strings.h>  
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

namespace socketops {

	int set_nonblock(int sockfd)
	{
		int flags = ::fcntl(sockfd, F_GETFL, 0);
		flags |= O_NONBLOCK;
		if (::fcntl(sockfd, F_SETFL, flags)==-1)
		{
			printf("cannot set nonblock:%s\n",strerror(errno));
			abort();
		}
		return sockfd;
	}

	int set_cloexec(int sockfd)
	{
		int flags = ::fcntl(sockfd, F_GETFD, 0);
		flags |= FD_CLOEXEC;
		if (::fcntl(sockfd, F_SETFD, flags)==-1)
		{
			printf("cannot set cloexec:%s\n",strerror(errno));
			abort();
		}
		return sockfd;
	}

	int set_nonblock_cloexec_(int sockfd)
	{
		return set_cloexec(set_nonblock(sockfd));
	}

	int mysocket_nonblock_()
	{
		int sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sockfd < 0)
		{
			return -1;
		}
		return set_nonblock_cloexec_(sockfd);
	}

	int mysocket_nonblock()
	{
		//int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
		int sockfd = mysocket_nonblock_();
		if (sockfd < 0)
		{
			printf("mysocket_nonblock cannot create socket\n");
			abort();
		}
		return sockfd;
	}

	int myconnect(int sockfd, const struct sockaddr_in& addr)
	{
		return ::connect(sockfd, sockaddr_cast(&addr), static_cast<socklen_t>(sizeof addr));
	}

	void mybind(int sockfd, const struct sockaddr_in& addr)
	{
		int ret = ::bind(sockfd, sockaddr_cast(&addr), static_cast<socklen_t>(sizeof addr));
		if (ret < 0)
		{
			printf("mybind failure\n");
			abort();
		}
	}

	void mylisten(int sockfd)
	{
		int ret = ::listen(sockfd, SOMAXCONN);
		if (ret < 0)
		{
			printf("mylisten failure\n");
			abort();
		}
	}

	int myaccept(int sockfd, struct sockaddr_in* addr)
	{
		socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);

		//int connfd = ::accept4(sockfd, sockaddr_cast(addr), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
		int connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
		set_nonblock_cloexec_(connfd);
		
		if (connfd < 0)
		{
			int savedErrno = errno;
			printf("accept4 error, err:%d, strerr:%s\n", savedErrno, strerror(savedErrno));
			switch (savedErrno)
			{
			case EAGAIN:
			case ECONNABORTED:
			case EINTR:
			case EPROTO: 
			case EPERM:
			case EMFILE: 
				// expected errors
				errno = savedErrno;
				break;
			case EBADF:
			case EFAULT:
			case EINVAL:
			case ENFILE:
			case ENOBUFS:
			case ENOMEM:
			case ENOTSOCK:
			case EOPNOTSUPP:
				// unexpected errors
				printf("unexpected error of accept4, err:%d\n", savedErrno);
				abort();
				break;
			default:
				printf("unknown error of accept4, err:%d\n", savedErrno);
				abort();
				break;
			}
		}
		return connfd;
	}

	int myread(int sockfd, void *buf, size_t count)
	{
		return ::read(sockfd, buf, count);
	}

	int myreadv(int sockfd, const struct iovec *iov, int iovcnt)
	{
		return ::readv(sockfd, iov, iovcnt);
	}

	int mywrite(int sockfd, const void *buf, size_t count)
	{
		return ::write(sockfd, buf, count);
	}

	void myclose(int sockfd)
	{
		if (::close(sockfd) < 0)
		{
			printf("close error\n");
		}
	}

	void myshutdownwrite(int sockfd)
	{
		if (::shutdown(sockfd, SHUT_WR) < 0)
		{
			printf("shutdown error\n");
		}
	}

	void toipport(char* buf, size_t size, const struct sockaddr_in& addr)
	{
		assert(size >= INET_ADDRSTRLEN);
		::inet_ntop(AF_INET, &addr.sin_addr, buf, static_cast<socklen_t>(size));
		size_t end = ::strlen(buf);
		uint16_t port = ntohs(addr.sin_port);//be16toh(addr.sin_port); 
		assert(size > end);
		snprintf(buf+end, size-end, ":%u", port);
	}

	void toip(char* buf, size_t size, const struct sockaddr_in& addr)
	{
		assert(size >= INET_ADDRSTRLEN);
		::inet_ntop(AF_INET, &addr.sin_addr, buf, static_cast<socklen_t>(size));
	}

	void fromipport(const char* ip, uint16_t port, struct sockaddr_in* addr)
	{
		addr->sin_family = AF_INET;
		addr->sin_port = htons(port);//htobe16(port);
		if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
		{
			printf("inet_pton error\n");
		}
	}

	int getsocketerror(int sockfd)
	{
		int optval;
		socklen_t optlen = static_cast<socklen_t>(sizeof optval);

		if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
		{
			return errno;
		}
		else
		{
			return optval;
		}
	}

	const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr)
	{
		return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
	}

	struct sockaddr* sockaddr_cast(struct sockaddr_in* addr)
	{
		return static_cast<struct sockaddr*>(implicit_cast<void*>(addr));
	}

	const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr)
	{
		return static_cast<const struct sockaddr_in*>(implicit_cast<const void*>(addr));
	}

	struct sockaddr_in* sockaddr_in_cast(struct sockaddr* addr)
	{
		return static_cast<struct sockaddr_in*>(implicit_cast<void*>(addr));
	}

	struct sockaddr_in getlocaladdr(int sockfd)
	{
		struct sockaddr_in localaddr;
		bzero(&localaddr, sizeof localaddr);
		socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
		if (::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0)
		{
			printf("getsockname error\n");
		}
		return localaddr;
	}

	struct sockaddr_in getpeeraddr(int sockfd)
	{
		struct sockaddr_in peeraddr;
		bzero(&peeraddr, sizeof peeraddr);
		socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
		if (::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0)
		{
			printf("getpeername error\n");
		}
		return peeraddr;
	}

	bool isselfconnect(int sockfd)
	{
		struct sockaddr_in localaddr = getlocaladdr(sockfd);
		struct sockaddr_in peeraddr = getpeeraddr(sockfd);
		return localaddr.sin_port == peeraddr.sin_port
			&& localaddr.sin_addr.s_addr == peeraddr.sin_addr.s_addr;
	}

}
