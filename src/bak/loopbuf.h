#ifndef LOOPBUF_HEADER
#define LOOPBUF_HEADER

#include <stdlib.h>
#include <string.h>
#include "define.h"

class loopbuf  
{
public:
	char* buf_;
	char* wptr_; 
	char* rptr_; 
	char* hptr_; 
	char* tptr_; 
	unsigned long count_;

public:
	loopbuf();
	loopbuf(unsigned long bufsize);
	virtual ~loopbuf();
	void init();
	void init(unsigned long bufsize);
	void reset();
	unsigned long put(char* buf, unsigned long size);
	unsigned long get(char* buf, unsigned long size);
	unsigned long peek(char* buf, unsigned long size);
	unsigned long erase(unsigned long size);
	unsigned long count(); 
	unsigned long freecount();
	unsigned long datacount();	
};

#endif 

