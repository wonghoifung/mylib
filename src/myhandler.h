#pragma once

#include "tcphandler.h"
#include "pack1.h"
#include "packparser1.h"
#include "define.h"
#include <map>
using namespace std;

enum
{
	st_connected = 0, 
	st_parsing, 
	st_closed,
};

class SocketHandler:public TcpHandler				
{
public:
	explicit SocketHandler(int nID);
	virtual ~SocketHandler(void);
public:
	//返回状态
	int GetChunkStatus(void){return m_nStatus;}

	int GetHandlerID(void){return m_nHandlerID;}

	//返回点分十进制的IP地址
	string  GetAddr(void){return m_addrremote;}		
	
	void *  GetUserData(){return m_pUserData;}	
	void    SetUserData(void *pUserData){	m_pUserData = pUserData;}
public:
	int     Send(NETOutputPacket *pPacket);
    //packet
    virtual int OnPacketComplete(NETInputPacket *);

private:
	// 协议解析
	virtual int OnParser(char *buf, int nLen);
	//连接关闭
	virtual int OnClose(void);
	//连接建立
	virtual int OnConnected(void);
    virtual int	ProcessOnTimerOut(int Timerid);

    //获取远端地址
	void GetRemoteAddr(void);
private:
	int    m_nStatus;				//Handler状态
	int    m_nHandlerID;			//socket句柄ID
	string m_addrremote;			//远端地址
	int    m_nPort;					//端口	
	void * m_pUserData;		        //用户数据
    IPacketParser * m_pParser;
};

