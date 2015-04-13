#ifndef PACKPARSER1_HEADER
#define PACKPARSER1_HEADER

#include <cstddef>

using namespace std;
class inpack1;
class tcphandler;

class ipackparser
{
protected:
	ipackparser() {};

public:
	ipackparser(tcphandler * pHandler):m_pHandler(pHandler) {};

	virtual ~ipackparser() {};

public:
	virtual int ParsePacket(const char * , const size_t ) = 0;

	static ipackparser * CreateObject(tcphandler * pObject);

protected:
	tcphandler* m_pHandler;
};

#endif //_ICHAT_PACKPARSER_H_

