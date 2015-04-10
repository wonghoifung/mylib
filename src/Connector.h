#ifndef __CONNECTOR_H__2011_4_27_
#define __CONNECTOR_H__2011_4_27_

#include "TcpHandler.h"
#include "TcpServer.h"
#include "SockerAPI.h"
#include <string>

class Connector 
{
public:
	Connector();
	virtual ~Connector();
	// �ҽӵ�TcpServer
	bool  Open( TcpServer* pServer );
	// ����
	bool  Connect( TcpHandler *pHandler,const string &strAddr,int port );
	bool  Connect( TcpHandler *pHandler,const NetAddr &addr );
protected:
	bool  Register(TcpHandler *pHandler);

protected:	
	TcpServer*	m_pNetServer;		// ע���Server	
};



#endif	//__CONNECTOR_H__2011_4_27_

