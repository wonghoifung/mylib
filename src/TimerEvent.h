#ifndef BOYAA_TIMER_EVENT_H_20110313
#define BOYAA_TIMER_EVENT_H_20110313

#include "timer.h"
#include "TimerOutEvent.h"

// ��ʱ����
class TimerEvent
{
public:
	TimerEvent(void);
	virtual ~TimerEvent(void);
	// ���ö�ʱ��ID
	void SetTimerId(int timer_id);
	// ������ʱ��
	void StartTimer(int sec, int usec = 0);
	// ֹͣ��ʱ��
	void StopTimer();
	// ���ö�ʱ��
	void ResetTimer();	
	// ���ö�ʱ���ص�����
	void SetTimeEventObj(TimerOutEvent * obj, int id=0);
public:
	virtual void OnTimer(int timer_id);

public:
	time_ev			m_ev;
protected:
	bool			m_bHaveStart;	// �Ƿ��ڼ�ʱ
	int				m_timeout;
	int				m_nId;
	TimerOutEvent	*m_TimeEvent;
};

#endif
