#ifndef SOCKETOPS_HEADER
#define SOCKETOPS_HEADER

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <string>
#include <assert.h>

namespace socketops
{
	int set_nonblock(int sockfd);
	int set_cloexec(int sockfd);
	void set_reuseaddr(int sockfd, bool on);
	void set_reuseport(int sockfd, bool on);
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

struct netaddr
{
	explicit netaddr(uint16_t port=0,bool loopbackonly=false) {
		bzero(&addr_, sizeof addr_);
		addr_.sin_family = AF_INET;
		in_addr_t ip = loopbackonly ? INADDR_LOOPBACK : INADDR_ANY;
		addr_.sin_addr.s_addr = htonl(ip);
		addr_.sin_port = htons(port);
	}

	netaddr(const std::string& ip, uint16_t port) {
		bzero(&addr_, sizeof addr_);
		addr_.sin_family = AF_INET;
		addr_.sin_port = htons(port);
		if (::inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr) <= 0) {
			printf("inet_pton error\n");
		}
	}

	netaddr(const struct sockaddr_in& addr) : addr_(addr) {}

	std::string toip() const {
		char buf[32] = {0};
		size_t size = sizeof buf;
		::inet_ntop(AF_INET, &addr_.sin_addr, buf, static_cast<socklen_t>(size));
		return buf;
	}

	std::string topipport() const {
		char buf[32] = {0};
		size_t size = sizeof buf;
		::inet_ntop(AF_INET, &addr_.sin_addr, buf, static_cast<socklen_t>(size));
		size_t end = ::strlen(buf);
		uint16_t port = ntohs(addr_.sin_port);
		assert(size > end);
		snprintf(buf+end, size-end, ":%u", port);
		return buf;
	}

	const struct sockaddr_in& getsockaddrin() const { return addr_; }

	void setsockaddrin(const struct sockaddr_in& addr) { addr_ = addr; }

	uint32_t ipnetendian() const { return addr_.sin_addr.s_addr; }

	uint16_t portnetendian() const { return addr_.sin_port; }

	static bool resolve(std::string hostname, netaddr* result);

	struct sockaddr_in addr_;
};

#endif
