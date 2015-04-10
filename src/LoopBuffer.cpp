
#ifdef WIN32
#include <Windows.h>
#endif

#include "LoopBuffer.h"

 //构造和析构
CLoopBuffer::CLoopBuffer()
{
	InitMember();
}
CLoopBuffer::CLoopBuffer(DWORD bufsize) 
{
	InitMember();
	Init(bufsize); 
}
CLoopBuffer::~CLoopBuffer() 
{
	if (buf_) 
	{
		free(buf_); 
		buf_ = 0;
	}
}
void CLoopBuffer::InitMember()
{
	buf_ = 0;	
	wptr_ = 0;
	rptr_ = 0;
	hptr_ = 0;
	tptr_ = 0;
	count_ = 0;
}
//初始化内存缓冲区大小和重置
void CLoopBuffer::Init(DWORD bufsize)
{		
	bufsize++;
	buf_ = (char*)malloc(bufsize);
	memset(buf_,0,bufsize);
	hptr_ = buf_;
	tptr_ = hptr_ + bufsize;
	wptr_ = rptr_ = hptr_;
	count_ = bufsize;
}
void CLoopBuffer::Reset()
{
	hptr_ = buf_;
	tptr_ = hptr_ + count_;
	wptr_ = rptr_ = hptr_;
}

//数据操作：Put, Get, Peek, Erase
//改变wptr_
DWORD CLoopBuffer::Put(char* buf, DWORD size)
{
	char* readptr	= rptr_;
	DWORD part	= tptr_ - wptr_;

	if (wptr_ >= readptr)
	{
		if (part >= size)
		{				
			memcpy(wptr_, buf, size);
			wptr_ += size;
			return size;
		}
		else
		{
			memcpy(wptr_, buf, part);
			buf += part;
			size -= part;
			if (size > DWORD(rptr_ - hptr_ - 1))
			{
				size = rptr_ - hptr_ - 1;
			}
			memcpy(hptr_, buf, size);
			wptr_ = hptr_ + size;
			return part + size;
		}
	}
	else 
	{
		if (size > DWORD(readptr - wptr_ - 1))
		{
			size = readptr - wptr_ - 1;
		}
		memcpy(wptr_, buf, size);
		wptr_ += size;
		return size;
	}
}

//会改变rptr_
DWORD CLoopBuffer::Get(char* buf, DWORD size) 
{
	char* writeptr	= wptr_;
	DWORD part	= tptr_ - rptr_;

	if (writeptr >= rptr_)
	{
		if (size > DWORD(writeptr - rptr_))
		{
			size = writeptr - rptr_;
		}
		memcpy(buf, rptr_, size);
		rptr_ += size;
		return size;
	}
	else
	{
		if (part >= size)
		{
			memcpy(buf, rptr_, size);
			rptr_ += size;
			return size;
		}
		else
		{
			memcpy(buf, rptr_, part);
			buf += part;
			size -= part;
			if (size > DWORD(writeptr - hptr_))
			{
				size = writeptr - hptr_;
			}
			memcpy(buf, hptr_, size);
			rptr_ = hptr_ + size;
			return part + size;
		}
	}
}
DWORD CLoopBuffer::Peek(char* buf, DWORD size)
{
	char* writeptr	= wptr_;
	DWORD part	= tptr_ - rptr_;

	if (writeptr >= rptr_)
	{
		if (size > DWORD(writeptr - rptr_))
		{
			size = writeptr - rptr_;
		}
		memcpy(buf, rptr_, size);
		return size;
	}
	else
	{
		if (part >= size)
		{
			memcpy(buf, rptr_, size);
			return size;
		}
		else
		{
			memcpy(buf, rptr_, part);
			buf += part;
			size -= part;
			if (size > DWORD(writeptr - hptr_))
			{
				size = writeptr - hptr_;
			}
			memcpy(buf, hptr_, size);
			return part + size;
		}
	}
}

DWORD CLoopBuffer::Erase(DWORD size)
{
	char* writeptr	= wptr_;
	DWORD part	= tptr_ - rptr_;

	if (writeptr >= rptr_)
	{
		if (size > DWORD(writeptr - rptr_))
		{
			size = writeptr - rptr_;
		}
		rptr_ += size;
		return size;
	}
	else
	{
		if (part >= size)
		{
			rptr_ += size;
			return size;
		}
		else
		{
			size -= part;
			if (size > DWORD(writeptr - hptr_))
				size = writeptr - hptr_;
			rptr_ = hptr_ + size;
			return part + size;
		}
	}
}

//空间大小，空闲空间大小，数据空间大小
DWORD CLoopBuffer::Count() 
{ 
	return count_ - 1; 
}
DWORD CLoopBuffer::FreeCount() 
{
	char* writeptr	= wptr_;
	char* readptr	= rptr_;
	if (writeptr >= readptr)
	{
		return count_ - (writeptr - readptr) -1;
	}
	else
	{
		return (readptr - writeptr) -1;
	}
}

DWORD CLoopBuffer::DataCount()
{
	char* writeptr	= wptr_;
	char* readptr	= rptr_;
	if (writeptr >= readptr)
	{
		return writeptr - readptr;
	}
	else
	{
		return count_ - (readptr - writeptr);
	}
}

