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
	bool open(tcpserver* pServer);
	bool connect(tcphandler* pHandler, const std::string& strAddr, int port);
protected:
	bool reg(tcphandler* pHandler);	
	tcpserver*	m_pNetServer;
};

#endif

