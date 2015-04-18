#ifndef CONNECTION_HEADER
#define CONNECTION_HEADER

#include <string>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>

#include "pack1.hpp"
#include "packparser1.h"
#include "loopbuf.h"

#define LOOPBUF_SIZE (64*1024)
#define RECVBUF_SIZE (32*1024)
#define SENDBUF_SIZE (32*1024)

typedef boost::function<void(connection*,inpack1*)> inpack1callback;
typedef boost::function<void(connection*)> connectedcallback;
typedef boost::function<void(connection*)> closedcallback;

class event_loop;

class connection : boost::noncopyable
{
public:
    connection(int fd, event_loop* eloop);
    ~connection();
    int onpackcomplete(inpack1* pack);
    int send(outpack1* pack);
    int send(const char* buf, int len);
    int socketwrite();
    bool needtowritesocket();

    int getfd() {return fd_;}
    void setfd(int fd) {fd_=fd;}
    uint32_t getfdindex() {return fdindex_;}
    void setfdindex(uint32_t idx) {fdindex_=idx;}
    void set_inpack1callback(inpack1callback cb) {inpack1cb_=cb;}
    void set_connectedcallback(connectedcallback cb) {connectedcb_=cb;}
    void set_closedcallback(closedcallback cb) {closedcb_=cb;}

    std::string get_localaddr();
    std::string get_peeraddr();
    event_loop* getevloop() {return evloop_;}

    int onconnected();
    int onclosed();
    int onread();
    int onwrite();

private:
    int fd_;
    uint32_t fdindex_; 
    
    ipackparser* parser1_;
    inpack1callback inpack1cb_;
    connectedcallback connectedcb_;
    closedcallback closedcb_;

    event_loop* evloop_;

    bool full_;
    loopbuf* sndloopbuf_;
    char sendbuf_[SENDBUF_SIZE];
    char recvbuf_[RECVBUF_SIZE];
};

bool isselfconnect(int sockfd);

#endif


