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

class myhandler:public tcphandler				
{
public:
	explicit myhandler(int nID);
	virtual ~myhandler(void);
public:
	//����״̬
	int getchunkstatus(void){return m_nStatus;}

	int gethandlerid(void){return m_nHandlerID;}

	//���ص��ʮ���Ƶ�IP��ַ
	string  getaddr(void){return m_addrremote;}		
	
	void *  getuserdata(){return m_pUserData;}	
	void    setuserdata(void *pUserData){	m_pUserData = pUserData;}
public:
	int     send_(outpack1 *pPacket);
    //packet
    virtual int onpackcomplete(inpack1 *);

private:
	// Э�����
	virtual int onparser(char *buf, int nLen);
	//���ӹر�
	virtual int onclose(void);
	//���ӽ���
	virtual int onconnected(void);
    virtual int	ontimeout(int Timerid);

    //��ȡԶ�˵�ַ
	void getremoteaddr(void);
private:
	int    m_nStatus;				//Handler״̬
	int    m_nHandlerID;			//socket���ID
	string m_addrremote;			//Զ�˵�ַ
	int    m_nPort;					//�˿�	
	void * m_pUserData;		        //�û�����
    ipackparser * m_pParser;
};

