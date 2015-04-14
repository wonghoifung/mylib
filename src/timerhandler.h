#ifndef TIMERHANDLER_HEADER
#define TIMERHANDLER_HEADER

class timerhandler
{
public:
    timerhandler(){}
    virtual ~timerhandler(){}
	virtual int ontimeout(int Timerid)=0;
};


#endif 


