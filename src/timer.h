#ifndef BOYAA_LINUX_TIMER_H
#define BOYAA_LINUX_TIMER_H

//#ifndef WIN32

#ifdef __cplusplus
extern "C" {
#endif

struct time_ev
{
	int time_id;
	void* timer;
	void* ptr;
	void (*callback)(void* ptr);
};

// 初始化定时器(在TcpServer初始化前初始化)
void init_timer(void);
// 投递定时器
int  start_timer(int sec, int usec, struct time_ev* ev);
// 删除定时器
int  stop_timer(struct time_ev* ev);
// 轮询定时器
void run_timer(void);

#ifdef __cplusplus
}
#endif

//#endif

#endif
