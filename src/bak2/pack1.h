#ifndef PACK1_HEADER
#define PACK1_HEADER

#include <time.h>
#include <assert.h>
#include <memory>
#include <stdarg.h>
#include <string.h>
#include <string>

class pack1
{
public:
	enum PACKETVER
	{
		SERVER_PACKET_DEFAULT_VER = 1,
		SERVER_PACKET_DEFAULT_SUBVER = 1
	};

	enum
	{
		PACKET_HEADER_SIZE = 9,
		PACKET_BUFFER_SIZE = 1024 * 24
	};

	pack1(void){ packsize_ = 0;}
	virtual ~pack1(void){}

	char *packet_buf(void)	{return pbuf_;}
	int packet_size(void)	{return packsize_;}

	short getcmd(void)
	{
		short nCmdType;
		_readHeader((char*)&nCmdType, sizeof(short), 2);
		return nCmdType;
	}

	char getversion(void)
	{
		char c;
		_readHeader(&c, sizeof(char), 4);
		return c;
	}

	char getsubversion(void)
	{
		char c;
		_readHeader(&c, sizeof(char), 5);
		return c;
	}
	
	short getbodylen(void)
	{
		short nLen;
		_readHeader((char*)&nLen, sizeof(short), 6);
		return nLen;
	}

	unsigned char getcheckcode(void)
	{
		unsigned char code;
		_readHeader((char*)&code, sizeof(unsigned char), 8);
		return code;
	}
	void _reset(void)
	{
		bufpos_ = PACKET_HEADER_SIZE;
		packsize_ = PACKET_HEADER_SIZE;
	}

protected:
	bool _copy(const void *pInBuf, int nLen)
	{
		if(nLen > PACKET_BUFFER_SIZE)
			return false;

		_reset();
		memcpy(pbuf_, pInBuf, nLen);
		packsize_ = nLen;
		assert(packsize_>=PACKET_HEADER_SIZE);
		return true;
	}

	void _begin(short nCmdType, char cVersion, char cSubVersion)
	{
		_reset();
		static char pszTag[] = "IC";
		_writeHeader(pszTag, sizeof(char)*2, 0);
		_writeHeader((char*)&nCmdType, sizeof(short), 2);
		_writeHeader(&cVersion, sizeof(char), 4);
		_writeHeader(&cSubVersion, sizeof(char), 5);
	}

	void _setbegin(short nCmdType)
	{
		_writeHeader((char*)&nCmdType, sizeof(short), 2);
	}

	void _end()
	{
		short nBody = static_cast<short>(packsize_ - PACKET_HEADER_SIZE);
		_writeHeader((char*)&nBody, sizeof(short), 6);
		unsigned char code = 0;
		_writeHeader((char*)&code, sizeof(unsigned char), 8);
	}

	void _oldend()
	{
		short nBody = static_cast<short>(packsize_ - PACKET_HEADER_SIZE);
		_writeHeader((char*)&nBody, sizeof(short), 6);
	}

	bool _read(char *pOut, int nLen)
	{
		if((nLen + bufpos_) > packsize_ || (nLen + bufpos_) > PACKET_BUFFER_SIZE )
			return false ;

		memcpy(pOut, pbuf_ + bufpos_, nLen);
		bufpos_ += nLen;
		return true;
	}

	bool _readdel(char *pOut, int nLen)
	{
		if(!_read(pOut, nLen))
			return false;
		memcpy(pbuf_ + bufpos_ - nLen, pbuf_ + bufpos_, PACKET_BUFFER_SIZE - bufpos_);
		bufpos_ -= nLen;
		packsize_ -= nLen;
		_end();
		return true;
	}

	void _readundo(int nLen)
	{
		bufpos_ -= nLen;
	}

	char *_readpoint(int nLen) 
	{
		if((nLen + bufpos_) > packsize_)
			return NULL; 
		char *p = &pbuf_[bufpos_];
		bufpos_ += nLen;
		return p;
	}

	bool _write(const char *pIn, int nLen)
	{
		if((packsize_ < 0) || ((nLen + packsize_) > PACKET_BUFFER_SIZE))
			return false ;
		memcpy(pbuf_+packsize_, pIn, nLen);
		packsize_ += nLen;
		return true;
	}

	bool _insert(const char *pIn, int nLen)
	{
		if((nLen + packsize_) > PACKET_BUFFER_SIZE)
			return false;
		memcpy(pbuf_+PACKET_HEADER_SIZE+nLen, pbuf_+PACKET_HEADER_SIZE, packsize_-PACKET_HEADER_SIZE);
		memcpy(pbuf_+PACKET_HEADER_SIZE, pIn, nLen);
		packsize_ += nLen;
		_end();
		return true;
	}

	bool _writezero(void)
	{
		if((packsize_ + 1) > PACKET_BUFFER_SIZE)
			return false ;
		memset(pbuf_+packsize_, '\0', sizeof(char)) ;
		packsize_ ++;
		return true;
	}

	void _readHeader(char *pOut, int nLen, int nPos)
	{
		if(nPos > 0 || nPos+nLen < PACKET_HEADER_SIZE)
		{
			memcpy(pOut, pbuf_+nPos, nLen) ;
		}
	}

	void _writeHeader(char *pIn, int nLen, int nPos)
	{
		if(nPos > 0 || nPos+nLen < PACKET_HEADER_SIZE)
		{
			memcpy(pbuf_+nPos, pIn, nLen) ;
		}
	}

private:
	char pbuf_[PACKET_BUFFER_SIZE];
	int packsize_;	
	int bufpos_;
};

class inpack1: public pack1
{
public:
	typedef pack1 base;

	int readint(void) {int nValue = -1; base::_read((char*)&nValue, sizeof(int)); return nValue;} 
	unsigned int readuint(void)	{unsigned int nValue = 0; base::_read((char*)&nValue, sizeof(unsigned int)); return nValue; }
	unsigned long readulong(void) {unsigned long nValue = 0; base::_read((char*)&nValue, sizeof(unsigned long)); return nValue;}
	int readintdel(void) {int nValue = -1; base::_readdel((char*)&nValue, sizeof(int)); return nValue;} 
	short readshort(void) {short nValue = -1; base::_read((char*)&nValue, sizeof(short)); return nValue;}
	unsigned char readbyte(void) {unsigned char nValue = 0; base::_read((char*)&nValue, sizeof(unsigned char)); return nValue;}

	bool readstring(char* pOutString, int nMaxLen)
	{
		int nLen = readint();
		if (nLen <= 0)
			return false;
		if(nLen > nMaxLen)
		{
			base::_readundo(sizeof(short));
			return false;
		}
		return base::_read(pOutString, nLen);
	}

	char* readchar(void)
	{
		int nLen = readint();
        if (nLen <= 0)
			return NULL;
		return base::_readpoint(nLen);
	}

	std::string readstring(void)
	{
		char* p = readchar();
		return (p == NULL ? "" : p);
	}

	int readbinary(char* pBuf, int nMaxLen)
	{
		int nLen = readint();
		if (nLen <= 0)
		{
			return -1;
		}

		if(nLen > nMaxLen)
		{
			base::_readundo(sizeof(int));
			return -1;
		}
		if(base::_read(pBuf, nLen))
			return nLen ;
		return 0;
	}

	void reset(void)
	{
		base::_reset();
	}

	bool copy(const void* pInBuf, int nLen)
	{
		return base::_copy(pInBuf, nLen);
	}

	bool writebody(const char* pIn, int nLen)
	{
		return base::_write(pIn, nLen);
	}

	void begin(short nCommand, char cVersion = SERVER_PACKET_DEFAULT_VER, char cSubVersion = SERVER_PACKET_DEFAULT_SUBVER)
	{
		base::_begin(nCommand, cVersion, cSubVersion);
	}

	void end(void)
	{
		base::_end();
	}

	char* tohexstring()
	{
		char * p = new char[packet_size() * 2 + 1];
		memset(p, 0, packet_size() * 2 + 1);
		for (int i = 0 ; i < this->packet_size(); ++i)
		{
			sprintf(&p[2 * i], "%02x", this->packet_buf()[i]);
		}

		return p;
	}

	void dump()
	{
		char * p = tohexstring();
		delete [] p;
		p = NULL;
	}
};

class outpack1: public pack1
{
	bool checkcode_;
public:
	outpack1(void){checkcode_ = false;}
	typedef pack1 base;

	bool writeint(int nValue) {return base::_write((char*)&nValue, sizeof(int));}
	bool writeuint(unsigned int nValue)	{return base::_write((char*)&nValue, sizeof(unsigned int));}
	bool writeulong(unsigned long nValue) {return base::_write((char*)&nValue, sizeof(unsigned long));}
	bool writebyte(unsigned char nValue) {return base::_write((char*)&nValue, sizeof(unsigned char));}
	bool writeshort(short nValue) {return base::_write((char*)&nValue, sizeof(short));}
	bool insertint(int nValue) {return base::_insert((char*)&nValue, sizeof(int));}
	bool insertbyte(unsigned char nValue) {return base::_insert((char*)&nValue, sizeof(unsigned char));}
	bool writestring(const char* pString)
	{
		int nLen = (int)strlen(pString) ;
		writeint(nLen + 1) ;
		return base::_write(pString, nLen) && base::_writezero();
	}

	bool writestring(const std::string& strDate)
	{
		int nLen = (int)strDate.size();
		writeint(nLen + 1) ;
		return base::_write(strDate.c_str(), nLen) && base::_writezero();
	}

	bool writebinary(const char* pBuf, int nLen)
	{
		writeint(nLen) ;
		return base::_write(pBuf, nLen) ;
	}

	bool copy(const void* pInBuf, int nLen)
	{
		return base::_copy(pInBuf, nLen);
	}

	bool copydata(const char* pInBuf, int nBufLen)
	{
		return base::_write(pInBuf, nBufLen);
	}

	void begin(short nCommand, char cVersion = SERVER_PACKET_DEFAULT_VER, char cSubVersion = SERVER_PACKET_DEFAULT_SUBVER)
	{
		base::_begin(nCommand, cVersion, cSubVersion);
		checkcode_ = false;
	}

	void end(void)
	{
		checkcode_ = false;
		base::_end();
	}

	void oldend(void)
	{
		checkcode_ = false;
		base::_oldend();
	}

	void setbegin(short nCommand)
	{
		base::_setbegin(nCommand);
	}

	void writecheckcode(unsigned char nValue)
	{
		base::_writeHeader((char*)&nValue, sizeof(unsigned char), 8); 
		checkcode_ = true;
	}

	bool hascheckcodebeenwritten(void)
	{
		return checkcode_;
	}

	void buildpack(short nCmdType, const char* pszFmt, ...)
	{
		begin(nCmdType);

		if (pszFmt == NULL)
		{
			end();
			return;
		}

		va_list ap; 
		va_start (ap, pszFmt); 
		const char* p = NULL;

		for (p= pszFmt; *p; p++) 
		{ 
			if (*p != '%') 
			{
				continue; 
			}

			switch (*++p) 
			{ 
			case 'd':	//int
				{
					int nVal= va_arg(ap, int);
					writeint(nVal);
					break;
				}
			case 'h':	//short
				{
					const int shVal = va_arg(ap, int);
					writeshort(static_cast<short>(shVal));
					break;
				}
			case 'u':	//unsigned long
				{
					unsigned long dwVal = va_arg(ap, unsigned long);
					writeulong(dwVal);
					break;
				}
			case 's':	//char*
				{
					char* pVal = va_arg(ap, char*);
					writestring(pVal);
					break;
				}
			}
		}
		end();
		va_end(ap);
	}
};

#endif 

