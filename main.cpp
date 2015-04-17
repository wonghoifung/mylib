#include "pack1.h"
#include "packparser1.h"
#include "connection.h"
#include "eventloop.h"
#include "tcpserver.h"

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
    tsvr.init(port);
    loop.run();

    return 0;
}


