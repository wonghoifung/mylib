#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <unistd.h>
#include "pack1.h"
#include "packparser1.h"
#include "connection.h"
#include "tcpclient.h"
#include "eventloop.h"

int main()
{
#if 1
    event_loop eloop;
    tcpclient tc(&eloop);
    if(tc.connect("127.0.0.1",9999)!=0) {
        printf("cannot connect to 127.0.0.1:9999\n");
        return 1;
    }
    for (int i=0; i<10; ++i) {
        const char* snd_buf = "helloworld";
        outpack1 out;
        out.begin(100+i);
        out.writeint(i);
        out.writestring(snd_buf);
        out.end();
        tc.send(&out);
        printf("send i:%d, msg:%s, packetsize:%d\n",i,snd_buf,out.packet_size());
    }
    eloop.run();
    return 0;
#else
    char *argv[] = {"127.0.0.1", "9999"};
    int connect_fd;
    int ret;
    char snd_buf[1024];
    int i=1;
    int port;
    int len;
    static struct sockaddr_in srv_addr;
    port=atoi(argv[1]);
    connect_fd=socket(PF_INET,SOCK_STREAM,0);
    if(connect_fd<0){
        perror("cannot create communication socket");
        return 1;
    }   
    memset(&srv_addr,0,sizeof(srv_addr));
    srv_addr.sin_family=AF_INET;
    srv_addr.sin_addr.s_addr=inet_addr(argv[0]);
    srv_addr.sin_port=htons(port);
    ret=connect(connect_fd,(struct sockaddr*)&srv_addr,sizeof(srv_addr));
    if(ret==-1){
        perror("cannot connect to the server");
        close(connect_fd);
        return 1;
    }
  
    //connection conn(connect_fd);


    memset(snd_buf,0,1024);
    while(1){
        write(STDOUT_FILENO,"input message:",14);
        bzero(snd_buf, 1024);
         
        len=read(STDIN_FILENO,snd_buf,1024);
        if(snd_buf[0]=='@')
            break;
        if(len>0) {
            snd_buf[strlen(snd_buf)-1]=0;
            outpack1 out;
            out.begin(88);
            out.writeint(i++);
            out.writestring(snd_buf);
            out.end();
            printf("send i:%d, msg:%s, packetsize:%d\n",i,snd_buf,out.packet_size());
            write(connect_fd,out.packet_buf(),out.packet_size());
            //write(connect_fd,snd_buf,len);
        }
        //len=read(connect_fd,snd_buf,len);
        //if(len>0)
        //    printf("Message from server: %s\n",snd_buf);
    }
   
    close(connect_fd);
#endif
    return 0;
}


