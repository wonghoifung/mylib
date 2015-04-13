
#ifndef TIMEROUTEVENT_H__
#define TIMEROUTEVENT_H__

// 定时器回调接口
class TimerOutEvent
{
public:
    TimerOutEvent(){}
    virtual ~TimerOutEvent(){}
	virtual int ontimeout(int Timerid)=0;
};


#endif //TIMEROUTEVENT_H__


