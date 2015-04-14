#ifndef MYHANDLER_HEADER
#define MYHANDLER_HEADER

#include <map>

#include "tcphandler.h"
#include "pack1.h"
#include "packparser1.h"
#include "define.h"

enum
{
	st_connected = 0, 
	st_parsing, 
	st_closed,
};

class myhandler : public tcphandler				
{
public:
	explicit myhandler(int nID);
	virtual ~myhandler(void);

	int getchunkstatus(void) {return hstate_;}
	int gethandlerid(void) {return hid_;}
	std::string  getaddr(void) {return remoteaddr_;}		
	
	void* getuserdata() {return userdata_;}	
	void setuserdata(void* pUserData) {userdata_ = pUserData;}

	int sendpack(outpack1* pPacket);
    virtual int onpackcomplete(inpack1*);

private:
	virtual int onparser(char *buf, int nLen);
	virtual int onclose(void);
	virtual int onconnected(void);
    virtual int	ontimeout(int Timerid);
	void getremoteaddr(void);

private:
	int hstate_;
	int hid_;
	std::string remoteaddr_;
	int port_;
	void* userdata_;
    ipackparser* parser_;
};

#endif
