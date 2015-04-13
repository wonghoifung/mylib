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
	//����״̬
	int GetChunkStatus(void){return m_nStatus;}

	int GetHandlerID(void){return m_nHandlerID;}

	//���ص��ʮ���Ƶ�IP��ַ
	string  GetAddr(void){return m_addrremote;}		
	
	void *  GetUserData(){return m_pUserData;}	
	void    SetUserData(void *pUserData){	m_pUserData = pUserData;}
public:
	int     Send(NETOutputPacket *pPacket);
    //packet
    virtual int OnPacketComplete(NETInputPacket *);

private:
	// Э�����
	virtual int OnParser(char *buf, int nLen);
	//���ӹر�
	virtual int OnClose(void);
	//���ӽ���
	virtual int OnConnected(void);
    virtual int	ProcessOnTimerOut(int Timerid);

    //��ȡԶ�˵�ַ
	void GetRemoteAddr(void);
private:
	int    m_nStatus;				//Handler״̬
	int    m_nHandlerID;			//socket���ID
	string m_addrremote;			//Զ�˵�ַ
	int    m_nPort;					//�˿�	
	void * m_pUserData;		        //�û�����
    IPacketParser * m_pParser;
};

