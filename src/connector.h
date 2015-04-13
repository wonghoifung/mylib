#ifndef CONNECTOR_HEADER
#define CONNECTOR_HEADER

#include "tcphandler.h"
#include "tcpserver.h"
#include "socketops.h"
#include <string>

class connector 
{
public:
	connector();
	virtual ~connector();
	bool open(TcpServer* pServer);
	bool connect(TcpHandler* pHandler, const string& strAddr, int port);
protected:
	bool reg(TcpHandler* pHandler);	
	TcpServer*	m_pNetServer;
};

#endif

