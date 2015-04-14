#ifndef TCPHANDLER_HEADER
#define TCPHANDLER_HEADER

#include "timerwrapper.h"
#include "timerhandler.h"
#include "loopbuf.h"
#include <map>

class tcpserver;
class inpack1;

#define LOOP_BUFFER_SIZE (1024*64)
#define RECV_BUFFER_SIZE (1024*32)		
#define SEND_BUFFER_SIZE (1024*32)		

class tcphandler : public timerhandler
{
public:
	tcphandler();	
	virtual ~tcphandler();	

	void setfd(int sock_fd);
	int getfd() const;

    uint32 get_fd_index(){ return fdindex_;}
    void set_fd_index(uint32 index){ fdindex_ = index;}
  
    bool getneeddel(){ return willdel_; }
    void setneeddel(bool bDel){ willdel_ = bDel; }

	tcpserver * server(void){return tcpserver_;}
	virtual void server(tcpserver *p){tcpserver_ = p;}

	int handle_connected();
	int handle_read();
	int handle_write();
	int handle_close();

	int sendpack(const char *buf, int nLen);
	bool writable();
	virtual int onpackcomplete(inpack1 *) = 0;

protected:
	virtual int onclose(void) {return -1;}	
	virtual int onconnected(void) {return 0;}	    
	virtual int onparser(char *, int) {return -1;}    
	virtual int	ontimeout(int Timerid) {return 0;};	

protected:
	int sockfd_;
    uint32 fdindex_;	    
    bool willdel_;       
    bool full_;  
	timerwrapper tcptimer_;
	tcpserver* tcpserver_;
	char recvbuf_[RECV_BUFFER_SIZE];	
	loopbuf* sendloopbuf_;
	char sendbuf_[SEND_BUFFER_SIZE];
};

#endif  


