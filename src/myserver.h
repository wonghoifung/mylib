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
	// 创建一个Handle连接
	virtual TcpHandler *    CreateHandler(void);
    virtual void            OnConnect(SocketHandler *pHandler );
    virtual void            OnDisconnect(SocketHandler *pHandler );
	// 处理OnTimer
	virtual int             ProcessOnTimer(SocketHandler *);
	// 处理消息包
	virtual int             ProcessPacket(NETInputPacket *pPacket, SocketHandler *pHandler, DWORD dwSessionID) = 0;
	// 查找Handler
	SocketHandler *         FindHandler(int nIndex);
	// 取一个HandlerID
	int                     GetUseID(void);
private:
    int                         m_nMaxID;		//分配给Handler的唯一标识
    map<int, SocketHandler*>	m_HandlerMap;
};

