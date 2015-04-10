#ifndef BOYAA_TIMER_EVENT_H_20110313
#define BOYAA_TIMER_EVENT_H_20110313

#include "timer.h"
#include "TimerOutEvent.h"

// 定时器类
class TimerEvent
{
public:
	TimerEvent(void);
	virtual ~TimerEvent(void);
	// 设置定时器ID
	void SetTimerId(int timer_id);
	// 开启定时器
	void StartTimer(int sec, int usec = 0);
	// 停止定时器
	void StopTimer();
	// 重置定时器
	void ResetTimer();	
	// 设置定时器回调对象
	void SetTimeEventObj(TimerOutEvent * obj, int id=0);
public:
	virtual void OnTimer(int timer_id);

public:
	time_ev			m_ev;
protected:
	bool			m_bHaveStart;	// 是否在计时
	int				m_timeout;
	int				m_nId;
	TimerOutEvent	*m_TimeEvent;
};

#endif
