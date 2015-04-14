#include "packparser1.h"
#include "pack1.h"
#include "myhandler.h"

class packparser1 :  public ipackparser
{
public:
	packparser1(tcphandler * pHandler):ipackparser(pHandler)
	{
		buf_ = pack_.packet_buf();
		version_ = inpack1::SERVER_PACKET_DEFAULT_VER;
		subversion_ = inpack1::SERVER_PACKET_DEFAULT_SUBVER;
		reset();
	}

	virtual ~packparser1(void){}

	int parsepack(const char *data, const size_t length)
	{
		int ret = -1; 
		size_t ndx = 0;
		while(ndx < length && hstate_ != REQ_ERROR) 
		{
			switch(hstate_)
			{
			case REQ_REQUEST:
			case REQ_HEADER:
				if(!read_header(data, length, ndx))
					break;
				ret = parse_header();
				if(ret != 0)
				{
					hstate_ = REQ_ERROR;
					break;
				}else
					hstate_ = REQ_BODY;
			case REQ_BODY:
				if(parse_body(data, length, ndx))
					hstate_ = REQ_DONE;
				break;
			default:
                {
                    return -1;
                }
			}
			if(hstate_ == REQ_ERROR)
            {
                reset();
            }
			if(hstate_ == REQ_DONE)
			{
				handler_->onpackcomplete(&pack_);
				this->reset();
			}
		}

		return 0; 
	}

protected:
	void reset(void)
	{
		hstate_ = REQ_REQUEST;
		packpos_ = 0;
		bodylen_ = 0;
		pack_.reset();
	}

private:
	bool read_header(const char *data, const size_t length, size_t & ndx)
	{
		if (0 == ndx)
		{
			packpos_ = 0;			
		}
		
		while(packpos_ < inpack1::PACKET_HEADER_SIZE && ndx < length)//
		{
			buf_[packpos_++] = data[ndx++];
		}
		if(packpos_ < inpack1::PACKET_HEADER_SIZE)
			return false;

		return true;
	}

	int parse_header(void)
	{
		if(buf_[0] != 'I' || buf_[1] != 'C')
			return -1;

		short nCmdType = pack_.getcmd();
		if(nCmdType < 0 || nCmdType >= 32000)
			return -2;

		if(pack_.getversion() != version_ || pack_.getsubversion() != subversion_)
			return -3;

		bodylen_ = pack_.getbodylen();
		if(bodylen_ < (inpack1::PACKET_BUFFER_SIZE - inpack1::PACKET_HEADER_SIZE))
			return 0;
		return -4;
	}

	bool parse_body(const char *data, const size_t length, size_t & ndx)
	{
		size_t nNeed = (bodylen_ + inpack1::PACKET_HEADER_SIZE) - packpos_;
		size_t nBuff = length - ndx;

		if(nNeed <= 0)
			return true;
		if(nBuff <= 0)
			return false;

		size_t nCopy = nBuff < nNeed ? nBuff : nNeed;
		if(!pack_.writebody(data + ndx,  static_cast<int>(nCopy)))
			return false;

		packpos_ += nCopy;
		ndx += nCopy;

		if(packpos_ < (bodylen_ + inpack1::PACKET_HEADER_SIZE))
			return false;

		return true;
	}

public:
	short version_;
	short subversion_;

private:
	int hstate_; 
	size_t	packpos_;
	size_t bodylen_;
	char* buf_;
	inpack1 pack_;
	enum REQSTATUS{	REQ_REQUEST=0, REQ_HEADER, REQ_BODY, REQ_DONE, REQ_PROCESS, REQ_ERROR };
};

ipackparser * ipackparser::createparser(tcphandler * pObject)
{
	return new packparser1(pObject);
}
