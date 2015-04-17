#ifndef TCPSERVER_HEADER
#define TCPSERVER_HEADER

#include <boost/noncopyable.hpp>
#include "connection.h"

class event_loop;

class tcpserver : boost::noncopyable
{
public:
    explicit tcpserver(event_loop* eloop);
    ~tcpserver();
    int init(int listenport);
    void set_inpack1callback(inpack1callback cb) {inpack1cb_=cb;}
    void set_connectedcallback(connectedcallback cb) {connectedcb_=cb;}
    void set_closedcallback(closedcallback cb) {closedcb_=cb;}
    inpack1callback get_inpack1callback() {return inpack1cb_;}
    connectedcallback get_connectedcallback() {return connectedcb_;}
    closedcallback get_closedcallback() {return closedcb_;}

private:
    int listenfd_;
    event_loop* evloop_;
    inpack1callback inpack1cb_;
    connectedcallback connectedcb_;
    closedcallback closedcb_;
};

#endif
