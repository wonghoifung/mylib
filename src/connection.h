#ifndef CONNECTION_HEADER
#define CONNECTION_HEADER

#include <boost/noncopyable.hpp>

#include "pack1.h"
#include "packparser1.h"

class connection : boost::noncopyable
{
public:
    connection(int fd);
    ~connection();
    int onpackcomplete(inpack1* pack);
    //void send(outpack1* pack);
//private:
    int fd_;
    ipackparser* parser1_;
};

#endif


