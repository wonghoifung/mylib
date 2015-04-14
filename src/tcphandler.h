#include "packparser1.h"

class tcphandler : public ipackparser
{
public:
    virtual int parsepack(const char* , const size_t ) { return 0; }
    virtual int onpackcomplete(inpack1 *) { return 0; }
};


