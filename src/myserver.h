#pragma once

#include "tcpserver.h"
#include "myhandler.h"
#include "timerwrapper.h"
#include <map>

using namespace std;

class myserver : public tcpserver
{
public:
	myserver(void);
	virtual ~myserver(void);
	virtual tcphandler* createhandler(void);
    virtual void onconnect(myhandler* pHandler);
    virtual void ondisconnect(myhandler* pHandler);
	virtual int ontimer(myhandler*);
	virtual int onpacket(inpack1* pPacket, myhandler* pHandler, DWORD dwSessionID) = 0;
	myhandler* gethandler(int nIndex);
	int gethid(void);

private:
    int m_nMaxID;
    map<int, myhandler*> m_HandlerMap;
};

