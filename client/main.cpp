#include <stdio.h>
#include <stdlib.h>
#include "pack1.h"
#include "packparser1.h"
#include "connection.h"
#include "tcpclient.h"
#include "eventloop.h"

void on_inpack1(connection* conn, inpack1* pack) {
    int cmd = pack->getcmd();
    int num1 =  pack->readint();
    std::string str1 = pack->readstring();
    printf("[from:%s] cmd:%d, num1:%d, str1:%s\n",
        conn->get_peeraddr().c_str(),
        cmd,num1,str1.c_str());
}

void on_connected(connection* conn) {
    printf("connected local:%s <-> peer:%s\n",
        conn->get_localaddr().c_str(),
        conn->get_peeraddr().c_str());
    for (int i=0; i<10; ++i) {
        const char* snd_buf = "helloworld";
        outpack1 out;
        out.begin(100+i);
        out.writeint(i);
        out.writestring(snd_buf);
        out.end();
        conn->send(&out);
        printf("send i:%d, msg:%s, packetsize:%d\n",i,snd_buf,out.packet_size());
    }

}

void on_closed(connection* conn) {
    printf("closed local:%s <-> peer:%s\n",
        conn->get_localaddr().c_str(),
        conn->get_peeraddr().c_str());
}

int main()
{
    event_loop eloop;
    tcpclient tc(&eloop);
    tc.set_inpack1callback(on_inpack1);
    tc.set_connectedcallback(on_connected);
    tc.set_closedcallback(on_closed);
    if(tc.connect("127.0.0.1",9999)!=0) {
        printf("cannot connect to 127.0.0.1:9999\n");
        return 1;
    }
    eloop.run();
    return 0;
}


