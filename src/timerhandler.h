
#ifndef TIMEROUTEVENT_H__
#define TIMEROUTEVENT_H__

// ��ʱ���ص��ӿ�
class TimerOutEvent
{
public:
    TimerOutEvent(){}
    virtual ~TimerOutEvent(){}
	virtual int ontimeout(int Timerid)=0;
};


#endif //TIMEROUTEVENT_H__


