#ifndef TCPCLIENT_HEADER
#define TCPCLIENT_HEADER

#include <boost/noncopyable.hpp>
#include "connection.h"

class event_loop;
class outpack1;

class tcpclient : boost::noncopyable
{
public:
    tcpclient(event_loop* eloop);
    ~tcpclient();
    int connect(const char* ip, int port);
    bool send(outpack1* pack);
    void set_inpack1callback(inpack1callback cb) {inpack1cb_=cb;}
    void set_connectedcallback(connectedcallback cb) {connectedcb_=cb;}
    void set_closedcallback(closedcallback cb) {closedcb_=cb;}

private:
    int connfd_;
    event_loop* evloop_;
    inpack1callback inpack1cb_;
    connectedcallback connectedcb_;
    closedcallback closedcb_;
};

#endif



