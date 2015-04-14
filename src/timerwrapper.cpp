#include <stdio.h>
#include <time.h>
#include "timerwrapper.h"
#include "log.h"

void handle_timeout(void* ptr)
{
	timerwrapper* sh = (timerwrapper*)ptr;
	sh->ontimeout(sh->timeev_.time_id);
}

timerwrapper::timerwrapper(void)
:havestart_(false)
,timeout_(0)
,id_(0)
,thandler_(0)
{
	timeev_.time_id	= 0;
	timeev_.timer = 0;
	timeev_.ptr	= this;
	timeev_.callback = handle_timeout;
}

timerwrapper::~timerwrapper(void)
{
	stoptimer();
}

void timerwrapper::starttimer(int sec, int usec)
{
	if(havestart_)
		stoptimer();
    if(sec <= 0)
    {
		log_error("invalid argument sec:%d",sec);
        return;
    }

	timeout_ = sec;
	if(start_timer(sec, usec, &timeev_) < 0)
    {
        log_error("start_timer failure");
        return;
    }
	havestart_ = true;
}

void timerwrapper::stoptimer()
{
	if(havestart_)
		stop_timer(&timeev_);
}

void timerwrapper::resettimer()
{
	stoptimer();
	starttimer(timeout_);
}

void timerwrapper::settimerid(int timer_id)
{
	timeev_.time_id = timer_id;
}

void timerwrapper::settimerhandler(timerhandler * obj, int id)
{
	id_ = id;
	thandler_ = obj;
}

void timerwrapper::ontimeout(int timer_id)
{
	havestart_ = false;

	if(thandler_ != 0)	
		thandler_->ontimeout(id_);
    else
        log_debug("timerhandler not set");  
}

