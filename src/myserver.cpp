#include "myserver.h"
#include "socketops.h"

using namespace std;

#ifndef WIN32
	#include <stdarg.h>
#endif

myserver::myserver(void)
:tcpserver()
{
	m_nMaxID = 0;
}

myserver::~myserver(void)
{
}
// 继承至ICHAT_TCP_Server, 创建新HANDLER时用
tcphandler * myserver::createhandler(void)
{
	myhandler *pNewHandler = NULL;
    int nHandlerID = gethid();
	pNewHandler = new myhandler(nHandlerID);
	return pNewHandler;
}
void  myserver::onconnect(myhandler *pHandler )
{
    int id = pHandler->gethandlerid();
    if(m_HandlerMap.find(id) == m_HandlerMap.end())
    {
        m_HandlerMap.insert(map<int, myhandler*>::value_type(id,pHandler));
    }
    else
    {
        log_debug("myserver::ProcessConnected Error %d\r\n", pHandler->gethandlerid());
        assert(false);
    }
}
void  myserver::ondisconnect(myhandler *pHandler )
{
    int id = pHandler->gethandlerid();
    map<int, myhandler*>::iterator iter = m_HandlerMap.find(id);
    if(iter != m_HandlerMap.end())
    {
        m_HandlerMap.erase(iter);
    }
    else
    {
        log_debug("myserver::ProcessClose Error %d\r\n",pHandler->gethandlerid());
        assert(false);
    }
}
// 超时
int myserver::ontimer(myhandler *pHandler)
{
    log_debug("connect 30s and no packet,disconnect \n");
    disconnect(pHandler);
	return 0;
}

// gethandler
myhandler * myserver::gethandler(int nIndex)
{
	map<int, myhandler*>::iterator iter = m_HandlerMap.find(nIndex);

	if(iter != m_HandlerMap.end())
	{
		return iter->second;
	}
	return NULL;
}

int myserver::gethid(void)
{
    ++m_nMaxID;
    while(m_HandlerMap.find(m_nMaxID) != m_HandlerMap.end())
    {
        ++m_nMaxID;
    }
	return m_nMaxID;
}
