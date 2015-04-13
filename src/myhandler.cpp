#include "myhandler.h"
#include "myserver.h"
#include <time.h>
//#include "EncryptDecrypt.h"
#include "socketops.h"

namespace
{
    static const string s_policystr ="<cross-domain-policy><allow-access-from domain=\"*\" to-ports=\"*\" /></cross-domain-policy>\0" ;// flash文件请求的固定回复
    static const int    s_DisNoMsgTime = 30;
}

myhandler::myhandler(int nID)
:tcphandler()
,m_nHandlerID(nID)
{	
	m_nStatus = -1;
	m_addrremote = "";
	m_nPort = 0;
	m_pUserData = NULL;
    m_pParser = NULL;
}

myhandler::~myhandler(void)
{
    if (m_pParser != NULL)
    {
        delete m_pParser;
        m_pParser = NULL;
    }
}

int myhandler::send_(outpack1 *pPacket)
{
    //CEncryptDecrypt::EncryptBuffer(*pPacket);
	return tcphandler::send_(pPacket->packet_buf(), pPacket->packet_size());
}
// onparser 协议解析
int myhandler::onparser(char *buf, int nLen)
{
	m_nStatus = st_parsing;
	m_TcpTimer.StopTimer();	//15内有请求取消定时
	if(nLen == 23 && buf[0] == '<' && buf[1] == 'p')
	{
		string policy = "<policy-file-request/>";
		for(int i=0; i<23; ++i)
		{
			if(buf[i] != policy[i])
            {
                log_debug("falsh fail ！\n");
                return -1;
            }
		}
		tcphandler::send_(s_policystr.c_str(), (int)s_policystr.size());
		return -1;
	}
    if(m_pParser == NULL)
        m_pParser = ipackparser::CreateObject(this);

	return m_pParser->ParsePacket(buf, nLen);
}

// 解析完成
int myhandler::onpackcomplete(inpack1 *pPacket)
{
// 	if(CEncryptDecrypt::DecryptBuffer(*pPacket) == -1)
//     {
//         log_debug("Decrypt fail \n");
//         return -1;
//     }
	myserver *pServer = (myserver *)this->server();
	return pServer->onpacket(pPacket, this, m_nHandlerID);
}
// onclose
int myhandler::onclose(void)
{
	m_nStatus = st_closed;	
    myserver *pServer = (myserver*)this->server();
    if(pServer != NULL)
        pServer->ondisconnect(this);
    return 0;
}
// onconnected
int myhandler::onconnected(void)
{
	m_nStatus = st_connected;
    myserver *pServer = (myserver*)this->server();
    if(pServer != NULL)
        pServer->onconnect(this);

	m_TcpTimer.StartTimer(s_DisNoMsgTime);   // s_DisNoMsgTime 秒没请求断开
	getremoteaddr();
    return 0;
}
int	myhandler::ontimeout(int Timerid)
{
    myserver *pServer = (myserver*)this->server();
    int nRet = pServer->ontimer(this);
    return nRet;
}

void myhandler::getremoteaddr(void)
{
	sockaddr_in remote_addr;
	memset(&remote_addr, 0, sizeof(remote_addr));
	int len = sizeof(remote_addr);
	if(getpeername(getfd(), reinterpret_cast<sockaddr *> (&remote_addr), (socklen_t*)&len) == 0)
	{
		m_addrremote = inet_ntoa(remote_addr.sin_addr);
		m_nPort = ntohs(remote_addr.sin_port);
	}
}
