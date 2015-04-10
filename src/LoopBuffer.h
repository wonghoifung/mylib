#ifndef AFX_LOOPBUFFER1_H__C1CEB05C_7A47_40FA_809E_C00D9922AD76__INCLUDED_
#define AFX_LOOPBUFFER1_H__C1CEB05C_7A47_40FA_809E_C00D9922AD76__INCLUDED_

#include <stdlib.h>
#include <string.h>
#include "define.h"

//ѭ��������
class CLoopBuffer  
{
public:
	char* buf_;
	char* wptr_; //дָ��
	char* rptr_; //��ָ��
	char* hptr_; //��������ͷָ��
	char* tptr_; //��������βָ��
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

