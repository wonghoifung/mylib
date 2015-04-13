#include "connector.h"
#include "socketops.h"

connector::connector()
{
	m_pNetServer = NULL;
}

connector::~connector()
{
}

bool connector::open(TcpServer* pServer)
{
	if(pServer == NULL)
		return false;
	m_pNetServer = pServer;
	return true;
}

bool connector::connect(TcpHandler* pHandler, const string& strAddr, int port)
{
	int sock_fd = socketops::myinit();
	if(pHandler == NULL || sock_fd < 0)
	{	
		return false;
	}
	if(socketops::myconnect(sock_fd, strAddr.c_str(), port) == 0)
	{	
        socketops::set_socketbuf(sock_fd, 16*1024);
        if(socketops::set_nonblock(sock_fd) < 0)
        {
			log_error("cannot set nonblock, fd:%d", sock_fd);
            socketops::myclose(sock_fd);
            return false;
        }
        if(socketops::set_keepalive(sock_fd) < 0)
        {
			log_error("cannot set keepalive, fd:%d", sock_fd);
            socketops::myclose(sock_fd);
            return false;
        }
        pHandler->SetFd(sock_fd);
		return reg(pHandler);			
	}
	socketops::myclose(sock_fd);	
	return false;
}

bool connector::reg(TcpHandler* pHandler)
{
	if(m_pNetServer == NULL)
		return false;
	return m_pNetServer->reg(pHandler);
}

