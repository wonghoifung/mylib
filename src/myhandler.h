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
	//返回状态
	int getchunkstatus(void){return m_nStatus;}

	int gethandlerid(void){return m_nHandlerID;}

	//返回点分十进制的IP地址
	string  getaddr(void){return m_addrremote;}		
	
	void *  getuserdata(){return m_pUserData;}	
	void    setuserdata(void *pUserData){	m_pUserData = pUserData;}
public:
	int     send_(outpack1 *pPacket);
    //packet
    virtual int onpackcomplete(inpack1 *);

private:
	// 协议解析
	virtual int onparser(char *buf, int nLen);
	//连接关闭
	virtual int onclose(void);
	//连接建立
	virtual int onconnected(void);
    virtual int	ontimeout(int Timerid);

    //获取远端地址
	void getremoteaddr(void);
private:
	int    m_nStatus;				//Handler状态
	int    m_nHandlerID;			//socket句柄ID
	string m_addrremote;			//远端地址
	int    m_nPort;					//端口	
	void * m_pUserData;		        //用户数据
    ipackparser * m_pParser;
};

