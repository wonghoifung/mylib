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

// ��ʼ����ʱ��(��TcpServer��ʼ��ǰ��ʼ��)
void init_timer(void);
// Ͷ�ݶ�ʱ��
int  start_timer(int sec, int usec, struct time_ev* ev);
// ɾ����ʱ��
int  stop_timer(struct time_ev* ev);
// ��ѯ��ʱ��
void run_timer(void);

#ifdef __cplusplus
}
#endif

//#endif

#endif
