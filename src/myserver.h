#ifndef MYSERVER_HEADER
#define MYSERVER_HEADER

#include "tcpserver.h"
#include "myhandler.h"
#include "timerwrapper.h"
#include <map>

class myserver : public tcpserver
{
public:
	myserver(void);
	virtual ~myserver(void);
	virtual tcphandler* createhandler(void);
    virtual void onconnect(myhandler* pHandler);
    virtual void ondisconnect(myhandler* pHandler);
	virtual int ontimer(myhandler*);
	virtual int onpacket(inpack1* pPacket, myhandler* pHandler, unsigned long dwSessionID) = 0;
	myhandler* gethandler(int nIndex);
	int gethid(void);

private:
    int maxid_;
	std::map<int, myhandler*> myhandlers_;
};

#endif

