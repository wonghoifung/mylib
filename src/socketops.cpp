#include "socketops.h"

#include <netinet/tcp.h>
#include <netinet/ip.h>

namespace socketops {

	std::vector<int> sessions;

	int maxnum = 0;

	// return code: 0 success, -1 failure
	int mylisten(int fd, int port)
	{
		struct sockaddr_in addr;
		memset(&addr , 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = htonl(INADDR_ANY);

		if ( 0 != bind(fd , (struct sockaddr*)&addr , sizeof(addr)) )
		{
			log_error("bind failure: %s", strerror(errno));		
			return -1;		
		}

		if(0 != listen(fd, 100000)) // 100000 make sense?
		{
			log_error("listen failure: %s", strerror(errno));		
			return -1;		
		}   

		return 0;
	}

	// return code: fd success, -1 failure
	int myaccept(int fd)
	{
		struct sockaddr_in clientaddr;
		socklen_t clilen = sizeof(clientaddr);
		int connfd = accept(fd, (struct sockaddr*)&clientaddr, &clilen);
		if(connfd < 0) { return -1; }
		log_debug("connection from %s ,port %d", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
		return connfd;
	}

	// return code: >0 success, <=0 failure
	int mysend(int fd, const char* buf, size_t len)
	{
		return send(fd, buf, len, 0);
	}

	// return code: >0 success, <=0 failure
	int myrecv(int fd, void* buf, size_t len)
	{
		return recv(fd, (char*)buf, len, 0);
	}

	void myclose(int fd)
	{
		close(fd);
	}

	// return code: 0 success, -1 failure
	int myconnect(int fd, const char* ip, int port)
	{
		struct sockaddr_in remote;
		memset(&remote, 0, sizeof(remote));
		remote.sin_family = AF_INET;
		remote.sin_port = htons(port);
		remote.sin_addr.s_addr = inet_addr(ip);
		if(0 != connect(fd, (struct sockaddr*)&remote, sizeof(remote)))
		{
			log_error("connect failure: %s", strerror(errno));
			return -1;
		}
		return 0;
	}

	// return code: 0 success, -1 failure
	int set_nonblock(int fd)
	{
		int opts = fcntl(fd, F_GETFL);
		if( opts < 0 )
			return -1;
		opts = opts | O_NONBLOCK;
		if(fcntl(fd, F_SETFL, opts) < 0)
			return -1;
		return 0;
	}

	// return code: 0 success, -1 failure
	int set_reuse(int fd)
	{
		int opt = 1;
		return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));	
	}

	// return code: 0 success, -1 failure
	int set_keepalive( int fd )
	{
		int keepAlive = 1; 
		int keepIdle = 60; 
		int keepInterval = 1; 
		int keepCount = 3; 

		int ret1 = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void*)&keepAlive, sizeof(keepAlive));
		int ret2 = setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle));
		int ret3 = setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, (void*)&keepInterval, sizeof(keepInterval));
		int ret4 = setsockopt(fd, SOL_TCP, TCP_KEEPCNT, (void*)&keepCount, sizeof(keepCount)); 

		if(ret1 == 0 && ret2 == 0 && ret3 == 0 && ret4 == 0)
			return 0;
		else
			return -1;
	}

	// return code: fd success, -1 failure
	int myinit(void)
	{
		int fd = socket(AF_INET, SOCK_STREAM, 0);
		if( 0 > fd)
		{
			log_error("socket failure: %s", strerror(errno));
			return -1;
		}
		return fd;
	}

	void set_socketbuf(int fd,int iSize)
	{
		int opt = iSize;
		socklen_t optlen = sizeof(opt);
		setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &opt, optlen);
		setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &opt, optlen);   
	}

	// return code: 0 success, -1 failure
	int myconnect_nonblock(int fd, const char* ip, int port)
	{
		struct sockaddr_in remote;
		memset(&remote, 0, sizeof(remote));
		remote.sin_family = AF_INET;
		remote.sin_port = htons(port);
		remote.sin_addr.s_addr = inet_addr(ip);
		if(fd > 0)
		{
			if(0 == socketops::set_nonblock(fd))
			{
				if(0 > connect(fd, (struct sockaddr*)&remote, sizeof(remote)))
				{
					if(errno != EINPROGRESS)
					{	
						return -1;
					} 
				}
				socketops::sessions.push_back(fd);
				socketops::maxnum++;
				return 0;
			}
			else
			{
				log_debug("set nonblock failure for fd:%d", fd);
			}
		}
		else
		{
			log_debug("invalid fd:%d", fd);
		}
		return -1;
	}

	// return code: fd success, -1 select failure, 0 timeout or connect done
	int wait_connect(int seconds)
	{
		if(socketops::maxnum <= 0)
		{
			log_debug("maxnum <= 0");
			socketops::maxnum = 0;
			return 0;
		}
		
		int maxfd = 0;
		int error = 0;
		socklen_t errlen = sizeof(error);
		
		fd_set readset;
		fd_set writeset;	
		
		FD_ZERO(&readset);
		FD_ZERO(&writeset);
		
		std::vector<int>::iterator iter = socketops::sessions.begin();
		for(; iter != socketops::sessions.end(); ++iter)
		{
			FD_SET(*iter, &readset);
			FD_SET(*iter, &writeset);
			if(maxfd < *iter)
			{
				maxfd = *iter;
			}
		}
		
		struct timeval tval;
		tval.tv_sec = seconds;
		tval.tv_usec = 0;
		
		int n = 0;
		if(0 == (n = select(maxfd + 1, &readset, &writeset, NULL, seconds ? &tval : NULL)))
		{
			for(iter = socketops::sessions.begin(); iter != socketops::sessions.end(); ++iter)
			{
				myclose(*iter);
			}		
			log_debug("select timeout");
			socketops::sessions.clear();
			socketops::maxnum = 0;
			return 0;
		}
		else if(n < 0)
		{
			log_debug("select error");
			return -1;
		}
		
		for(iter = socketops::sessions.begin(); iter != socketops::sessions.end(); )
		{
			if(FD_ISSET(*iter, &readset) || FD_ISSET(*iter, &writeset))
			{
				if(getsockopt(*iter, SOL_SOCKET, SO_ERROR, (char*)&error, &errlen) < 0)
				{
					log_debug("cannot connect to server, fd:%d", *iter);
					iter = socketops::sessions.erase(iter);
					--socketops::maxnum;
					myclose(*iter);
				}
				else
				{
					int rtfd = *iter;
					iter = socketops::sessions.erase(iter);
					--socketops::maxnum;
					if(error != 0)
					{
						log_debug("cannot connect to server, fd:%d error:%s", *iter, strerror(error));
					}
					else
					{
						log_debug("connect success, fd:%d", *iter);
						return rtfd;
					}				
				}
				continue;
			}
			++iter;
		}
		return 0;
	}

}
