
#include <stdio.h>
#include "timerwrapper.h"
#include "log.h"
#include <time.h>
// 回调函数
void handle_timeout(void* ptr)
{
	TimerEvent* sh = (TimerEvent*)ptr;
#ifdef WIN32
	stop_timer(&sh->m_ev);			// 放在OnTimer前面，是为了防止在OnTimer里面StartTimer
#endif
	sh->OnTimer(sh->m_ev.time_id);
}

TimerEvent::TimerEvent(void)
:m_bHaveStart(false)
,m_timeout(0)
,m_nId(0)
,m_TimeEvent(0)
{
	m_ev.time_id	= 0;
	m_ev.timer		= 0;
	m_ev.ptr		= this;
	m_ev.callback	= handle_timeout;
}

TimerEvent::~TimerEvent(void)
{
	StopTimer();
}
// 开启定时器
void TimerEvent::StartTimer(int sec, int usec)
{
	if(m_bHaveStart)
		StopTimer();
    if(sec <= 0)
    {
        log_error("the timer is error %d ",sec);
        return;
    }

	m_timeout = sec;
	if(start_timer(sec, usec, &m_ev) < 0)
    {
        log_error("start_timer fail \n");
        return;
    }
	m_bHaveStart = true;
}
// 停止定时器
void TimerEvent::StopTimer()
{
	if(m_bHaveStart)
		stop_timer(&m_ev);
}
// 重置定时器
void TimerEvent::ResetTimer()
{
	StopTimer();
	StartTimer(m_timeout);
}
// 设置定时器ID
void TimerEvent::SetTimerId(int timer_id)
{
	m_ev.time_id = timer_id;
}
// 设置定时器回调对象
void TimerEvent::SetTimeEventObj(TimerOutEvent * obj, int id)
{
	m_nId = id;
	m_TimeEvent = obj;
}
// 定时器响应函数(允许重载)
void TimerEvent::OnTimer(int timer_id)
{
	m_bHaveStart = false;

	if(m_TimeEvent != 0)	
		m_TimeEvent->ProcessOnTimerOut(m_nId);
    else
        log_debug("Timeobj is null \n");  
}

