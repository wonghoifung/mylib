#ifndef PACKPARSER1_HEADER
#define PACKPARSER1_HEADER

#include <cstddef>

class inpack1;
class connection;

class ipackparser
{
protected:
	ipackparser() {};

public:
	ipackparser(connection* pHandler):handler_(pHandler) {}
	virtual ~ipackparser() {}
	virtual int parsepack(const char*, const size_t) = 0;
	static ipackparser* createparser(connection* pObject);

protected:
	connection* handler_;
};

#endif 

