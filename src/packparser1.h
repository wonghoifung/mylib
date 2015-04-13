#ifndef _PACKPARSER_H_
#define _PACKPARSER_H_

#include <cstddef>

using namespace std;
class NETInputPacket;
class TcpHandler;

class IPacketParser
{
protected:
	IPacketParser() {};

public:
	IPacketParser(TcpHandler * pHandler):m_pHandler(pHandler) {};

	virtual ~IPacketParser() {};

public:
	virtual int ParsePacket(const char * , const size_t ) = 0;

	static IPacketParser * CreateObject(TcpHandler * pObject);

protected:
	TcpHandler* m_pHandler;
};

#endif //_ICHAT_PACKPARSER_H_

