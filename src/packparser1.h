#ifndef PACKPARSER1_HEADER
#define PACKPARSER1_HEADER

#include <cstddef>

class inpack1;
class tcphandler;

class ipackparser
{
protected:
	ipackparser() {};

public:
	ipackparser(tcphandler* pHandler):handler_(pHandler) {}
	virtual ~ipackparser() {}
	virtual int parsepack(const char* , const size_t ) = 0;
	static ipackparser* createparser(tcphandler* pObject);

protected:
	tcphandler* handler_;
};

#endif 

