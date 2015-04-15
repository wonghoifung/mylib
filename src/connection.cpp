#include "connection.h"
#include <stdio.h>

connection::connection(int fd):fd_(fd),parser1_(ipackparser::createparser(this))
{
}

connection::~connection()
{
    if(parser1_) {
        delete parser1_;
        parser1_ = NULL;
    }
}

int connection::onpackcomplete(inpack1* pack)
{
    printf("cmd: %d\n", pack->getcmd());
    printf("int: %d\n", pack->readint());
    printf("str: %s\n", pack->readstring().c_str());
    return 0;
}


