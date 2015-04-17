#ifndef TCPCLIENT_HEADER
#define TCPCLIENT_HEADER

#include <boost/noncopyable.hpp>

class event_loop;
class outpack1;

class tcpclient : boost::noncopyable
{
public:
    tcpclient(event_loop* eloop);
    ~tcpclient();
    int connect(const char* ip, int port);
    bool send(outpack1* pack);

private:
    int connfd_;
    event_loop* evloop_;
};

#endif



