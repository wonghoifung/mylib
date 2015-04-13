#include "myserver.h"
#include "socketops.h"

using namespace std;

#ifndef WIN32
	#include <stdarg.h>
#endif

SocketServer::SocketServer(void)
:TcpServer()
{
	m_nMaxID = 0;
}

SocketServer::~SocketServer(void)
{
}
// 继承至ICHAT_TCP_Server, 创建新HANDLER时用
TcpHandler * SocketServer::CreateHandler(void)
{
	SocketHandler *pNewHandler = NULL;
    int nHandlerID = GetUseID();
	pNewHandler = new SocketHandler(nHandlerID);
	return pNewHandler;
}
void  SocketServer::OnConnect(SocketHandler *pHandler )
{
    int id = pHandler->GetHandlerID();
    if(m_HandlerMap.find(id) == m_HandlerMap.end())
    {
        m_HandlerMap.insert(map<int, SocketHandler*>::value_type(id,pHandler));
    }
    else
    {
        log_debug("SocketServer::ProcessConnected Error %d\r\n", pHandler->GetHandlerID());
        assert(false);
    }
}
void  SocketServer::OnDisconnect(SocketHandler *pHandler )
{
    int id = pHandler->GetHandlerID();
    map<int, SocketHandler*>::iterator iter = m_HandlerMap.find(id);
    if(iter != m_HandlerMap.end())
    {
        m_HandlerMap.erase(iter);
    }
    else
    {
        log_debug("SocketServer::ProcessClose Error %d\r\n",pHandler->GetHandlerID());
        assert(false);
    }
}
// 超时
int SocketServer::ProcessOnTimer(SocketHandler *pHandler)
{
    log_debug("connect 30s and no packet,disconnect \n");
    DisConnect(pHandler);
	return 0;
}

// FindHandler
SocketHandler * SocketServer::FindHandler(int nIndex)
{
	map<int, SocketHandler*>::iterator iter = m_HandlerMap.find(nIndex);

	if(iter != m_HandlerMap.end())
	{
		return iter->second;
	}
	return NULL;
}

int SocketServer::GetUseID(void)
{
    ++m_nMaxID;
    while(m_HandlerMap.find(m_nMaxID) != m_HandlerMap.end())
    {
        ++m_nMaxID;
    }
	return m_nMaxID;
}
