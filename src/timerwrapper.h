#ifndef TIMERWRAPPER_HEADER
#define TIMERWRAPPER_HEADER

#include "timer.h"
#include "timerhandler.h"

class timerwrapper
{
public:
	timerwrapper(void);
	virtual ~timerwrapper(void);
	void settimerid(int timer_id);
	void starttimer(int sec, int usec = 0);
	void stoptimer();
	void resettimer();	
	void settimerhandler(timerhandler* obj, int id=0);
	virtual void ontimeout(int timer_id);

public:
	time_ev	timeev_;

protected:
	bool havestart_;
	int	timeout_;
	int	id_;
	timerhandler* thandler_;
};

#endif
