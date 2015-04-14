#include "myserver.h"
#include "socketops.h"
#include <stdarg.h>

myserver::myserver(void):tcpserver()
{
	maxid_ = 0;
}

myserver::~myserver(void)
{
}

tcphandler* myserver::createhandler(void)
{
	myhandler* pNewHandler = NULL;
    int nHandlerID = gethid();
	pNewHandler = new myhandler(nHandlerID);
	return pNewHandler;
}

void myserver::onconnect(myhandler* pHandler)
{
    int id = pHandler->gethandlerid();
    if(myhandlers_.find(id) == myhandlers_.end())
    {
		myhandlers_.insert(std::map<int, myhandler*>::value_type(id,pHandler));
    }
    else
    {
		log_debug("myserver::onconnect error, hid:%d", pHandler->gethandlerid());
        assert(false);
    }
}

void myserver::ondisconnect(myhandler* pHandler)
{
    int id = pHandler->gethandlerid();
    std::map<int, myhandler*>::iterator iter = myhandlers_.find(id);
    if(iter != myhandlers_.end())
    {
        myhandlers_.erase(iter);
    }
    else
    {
		log_debug("myserver::ondisconnect error, hid:%d",pHandler->gethandlerid());
        assert(false);
    }
}

int myserver::ontimer(myhandler* pHandler)
{
	log_debug("disconnect because it has been idle for a long time, hid:%d",pHandler->gethandlerid());
    disconnect(pHandler);
	return 0;
}

myhandler* myserver::gethandler(int nIndex)
{
	std::map<int, myhandler*>::iterator iter = myhandlers_.find(nIndex);
	if(iter != myhandlers_.end())
	{
		return iter->second;
	}
	return NULL;
}

int myserver::gethid(void)
{
    ++maxid_;
    while(myhandlers_.find(maxid_) != myhandlers_.end())
    {
        ++maxid_;
    }
	return maxid_;
}
