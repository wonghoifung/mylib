#ifndef AFX_LOOPBUFFER1_H__C1CEB05C_7A47_40FA_809E_C00D9922AD76__INCLUDED_
#define AFX_LOOPBUFFER1_H__C1CEB05C_7A47_40FA_809E_C00D9922AD76__INCLUDED_

#include <stdlib.h>
#include <string.h>
#include "define.h"

//循环缓冲区
class CLoopBuffer  
{
public:
	char* buf_;
	char* wptr_; //写指针
	char* rptr_; //读指针
	char* hptr_; //缓冲区的头指针
	char* tptr_; //缓冲区的尾指针
	DWORD count_;
public:
	CLoopBuffer();
	CLoopBuffer(DWORD bufsize);
	virtual ~CLoopBuffer();
	void InitMember();
	void Init(DWORD bufsize);
	void Reset();
	DWORD Put(char* buf, DWORD size);
	DWORD Get(char* buf, DWORD size);
	DWORD Peek(char* buf, DWORD size);
	DWORD Erase(DWORD size);
	DWORD Count(); 
	DWORD FreeCount();
	DWORD DataCount();	
};

#endif // !defined(AFX_LOOPBUFFER1_H__C1CEB05C_7A47_40FA_809E_C00D9922AD76__INCLUDED_)

