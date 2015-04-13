#pragma once
//#include "ichatlib.h"
#include "tcpserver.h"
#include "myhandler.h"
#include "timerwrapper.h"
#include <map>

using namespace std;

class SocketServer:public TcpServer
{
public:
	SocketServer(void);
	virtual ~SocketServer(void);

public:
	// ����һ��Handle����
	virtual TcpHandler *    CreateHandler(void);
    virtual void            OnConnect(SocketHandler *pHandler );
    virtual void            OnDisconnect(SocketHandler *pHandler );
	// ����OnTimer
	virtual int             ProcessOnTimer(SocketHandler *);
	// ������Ϣ��
	virtual int             ProcessPacket(NETInputPacket *pPacket, SocketHandler *pHandler, DWORD dwSessionID) = 0;
	// ����Handler
	SocketHandler *         FindHandler(int nIndex);
	// ȡһ��HandlerID
	int                     GetUseID(void);
private:
    int                         m_nMaxID;		//�����Handler��Ψһ��ʶ
    map<int, SocketHandler*>	m_HandlerMap;
};

