#include <time.h>
#include "myhandler.h"
#include "myserver.h"
#include "socketops.h"

static const int maxidletime = 30;

myhandler::myhandler(int nID)
:tcphandler()
,hid_(nID)
{	
	hstate_ = -1;
	remoteaddr_ = "";
	port_ = 0;
	userdata_ = NULL;
    parser_ = NULL;
}

myhandler::~myhandler(void)
{
    if (parser_ != NULL)
    {
        delete parser_;
        parser_ = NULL;
    }
}

int myhandler::sendpack(outpack1* pPacket)
{
	return tcphandler::sendpack(pPacket->packet_buf(), pPacket->packet_size());
}

int myhandler::onparser(char* buf, int nLen)
{
	hstate_ = st_parsing;
	tcptimer_.stoptimer();	
    if(parser_ == NULL)
        parser_ = ipackparser::createparser(this);
	return parser_->parsepack(buf, nLen);
}

int myhandler::onpackcomplete(inpack1* pPacket)
{
	myserver* pServer = (myserver*)this->server();
	return pServer->onpacket(pPacket, this, hid_);
}

int myhandler::onclose(void)
{
	hstate_ = st_closed;	
    myserver* pServer = (myserver*)this->server();
    if(pServer != NULL)
        pServer->ondisconnect(this);
    return 0;
}

int myhandler::onconnected(void)
{
	hstate_ = st_connected;
    myserver *pServer = (myserver*)this->server();
    if(pServer != NULL)
        pServer->onconnect(this);
	tcptimer_.starttimer(maxidletime); 
	getremoteaddr();
    return 0;
}

int	myhandler::ontimeout(int Timerid)
{
    myserver* pServer = (myserver*)this->server();
    int nRet = pServer->ontimer(this);
    return nRet;
}

void myhandler::getremoteaddr(void)
{
	sockaddr_in remote_addr;
	memset(&remote_addr, 0, sizeof(remote_addr));
	int len = sizeof(remote_addr);
	if(getpeername(getfd(), reinterpret_cast<sockaddr*>(&remote_addr), (socklen_t*)&len) == 0)
	{
		remoteaddr_ = inet_ntoa(remote_addr.sin_addr);
		port_ = ntohs(remote_addr.sin_port);
	}
}
