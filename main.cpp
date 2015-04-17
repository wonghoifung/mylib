#include "pack1.h"
#include "packparser1.h"
#include "connection.h"
#include "eventloop.h"
#include "tcpserver.h"

void on_inpack1(connection* conn, inpack1* pack) {
}

void on_connected(connection* conn) {
}

void on_closed(connection* conn) {
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


