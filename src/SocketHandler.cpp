#include "SocketHandler.h"
#include "SocketServer.h"
#include <time.h>
//#include "EncryptDecrypt.h"
#include "SockerAPI.h"

namespace
{
    static const string s_policystr ="<cross-domain-policy><allow-access-from domain=\"*\" to-ports=\"*\" /></cross-domain-policy>\0" ;// flash文件请求的固定回复
    static const int    s_DisNoMsgTime = 30;
}

SocketHandler::SocketHandler(int nID)
:TcpHandler()
,m_nHandlerID(nID)
{	
	m_nStatus = -1;
	m_addrremote = "";
	m_nPort = 0;
	m_pUserData = NULL;
    m_pParser = NULL;
}

SocketHandler::~SocketHandler(void)
{
    if (m_pParser != NULL)
    {
        delete m_pParser;
        m_pParser = NULL;
    }
}

int SocketHandler::Send(NETOutputPacket *pPacket)
{
    //CEncryptDecrypt::EncryptBuffer(*pPacket);
	return TcpHandler::Send(pPacket->packet_buf(), pPacket->packet_size());
}
// OnParser 协议解析
int SocketHandler::OnParser(char *buf, int nLen)
{
	m_nStatus = REQUEST;
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
		TcpHandler::Send(s_policystr.c_str(), (int)s_policystr.size());
		return -1;
	}
    if(m_pParser == NULL)
        m_pParser = IPacketParser::CreateObject(this);

	return m_pParser->ParsePacket(buf, nLen);
}

// 解析完成
int SocketHandler::OnPacketComplete(NETInputPacket *pPacket)
{
// 	if(CEncryptDecrypt::DecryptBuffer(*pPacket) == -1)
//     {
//         log_debug("Decrypt fail \n");
//         return -1;
//     }
	SocketServer *pServer = (SocketServer *)this->server();
	return pServer->ProcessPacket(pPacket, this, m_nHandlerID);
}
// OnClose
int SocketHandler::OnClose(void)
{
	m_nStatus = CLOSE;	
    SocketServer *pServer = (SocketServer*)this->server();
    if(pServer != NULL)
        pServer->OnDisconnect(this);
    return 0;
}
// OnConnected
int SocketHandler::OnConnected(void)
{
	m_nStatus = CONNECT;
    SocketServer *pServer = (SocketServer*)this->server();
    if(pServer != NULL)
        pServer->OnConnect(this);

	m_TcpTimer.StartTimer(s_DisNoMsgTime);   // s_DisNoMsgTime 秒没请求断开
	GetRemoteAddr();
    return 0;
}
int	SocketHandler::ProcessOnTimerOut(int Timerid)
{
    SocketServer *pServer = (SocketServer*)this->server();
    int nRet = pServer->ProcessOnTimer(this);
    return nRet;
}

void SocketHandler::GetRemoteAddr(void)
{
	sockaddr_in remote_addr;
	memset(&remote_addr, 0, sizeof(remote_addr));
	int len = sizeof(remote_addr);
	if(getpeername(GetFd(), reinterpret_cast<sockaddr *> (&remote_addr), (socklen_t*)&len) == 0)
	{
		m_addrremote = inet_ntoa(remote_addr.sin_addr);
		m_nPort = ntohs(remote_addr.sin_port);
	}
}
