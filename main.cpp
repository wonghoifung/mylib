#include "pack1.hpp"
#include "packparser1.h"
#include "connection.h"
#include "eventloop.h"
#include "tcpserver.h"

void on_inpack1(connection* conn, inpack1* pack) {
    int cmd = pack->getcmd();
    int num1 =  pack->readint();
    std::string str1 = pack->readstring();
    printf("[from:%s] cmd:%d, num1:%d, str1:%s\n",
        conn->get_peeraddr().c_str(),
        cmd,num1,str1.c_str());

    outpack1 out;
    out.begin(99);
    out.writeint(999);
    out.writestring("helloclient");
    out.end();
    conn->send(&out);
}

void on_connected(connection* conn) {
    printf("connected local:%s <-> peer:%s\n",
        conn->get_localaddr().c_str(),
        conn->get_peeraddr().c_str());
}

void on_closed(connection* conn) {
    printf("closed local:%s <-> peer:%s\n",
        conn->get_localaddr().c_str(),
        conn->get_peeraddr().c_str());
}

int main( int argc, char* argv[] )
{
    if( argc <= 2 )
    {
        printf( "usage: %s ip_address port_number\n", basename( argv[0] ) );
        return 1;
    }
    //const char* ip = argv[1];
    int port = atoi( argv[2] );

    event_loop loop;
    tcpserver tsvr(&loop);
    tsvr.set_inpack1callback(on_inpack1);
    tsvr.set_connectedcallback(on_connected);
    tsvr.set_closedcallback(on_closed);
    tsvr.init(port);
    loop.run();

    return 0;
}


