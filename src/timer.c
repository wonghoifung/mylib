
#include <stdio.h>
#include "timer.h"
#include "llist.h"	

#ifndef WIN32
	#include "slowtimer.c"
	#include "plex.h"
	plex_t timer_plex;
#endif 

#ifndef WIN32	//linux platform
// ��ʼ����ʱ��
void init_timer()
{
	stimer_init();
	plex_init(&timer_plex, sizeof(stimer_t));
	run_timer();
}
// Linux��ʱ���ص�����
void on_timer(void* ctx)
{
	struct time_ev* ev = ctx;
	//�ͷŶ�ʱ��
	stop_timer(ev);	    
	ev->callback(ev->ptr);
}
// Ͷ��һ����ʱ��
int start_timer(int sec, int usec, struct time_ev* ev)
{
	void* timer = 0;

	//timer malloc
	timer = plex_alloc(&timer_plex);
	if (timer == NULL) {
		if (0 != plex_expand(&timer_plex, 64, &sys_malloc, 0)) {
			return -1;
		}
		timer = plex_alloc(&timer_plex);
		assert(timer);
	}

	//add timer
	stimer_set(timer, sec*1000 + usec / 1000, &on_timer, ev);
	if (0 != stimer_add(timer)) {
		plex_free(&timer_plex, timer);
		return -1;
	};
	ev->timer = timer;
	return 0;
}
// ɾ��һ����ʱ��
int stop_timer(struct time_ev* ev)
{
	if (ev->timer) {
		stimer_del(ev->timer);//�����Ƿ�ɹ����ö�ʱ�������������У��ʿ���ɾ���ڴ�
		plex_free(&timer_plex, ev->timer);
		ev->timer = NULL;
	}
	return 0;
}
// ��ѯ��ʱ��
void run_timer()
{
	stimer_collect();
}

#else	//win32 platform
// �����Ƿ�pELE �к��� key
int find(ELE* pELE, void* key)
{
    struct time_ev* ev1 = (struct time_ev*)key, *ev2 = (struct time_ev*)(pELE->k);

    if(ev2 == ev1)
		return 1;
	else
		return 0;
}
// ����Ƿ�ʱ
int travel(ELE* pELE, void* key)
{
	struct time_ev* ev = (struct time_ev*)(pELE->k);
	(*pELE).i -= 1;
	if((*pELE).i <= 0)
	{
		ev->callback(ev->ptr);
	}
		
	return 0;
}
// ��ʼ����ʱ��
void init_timer(void)
{
 	if(lhandle)
 		llist_destory(lhandle);
 
 	lhandle = llist_creat();
 	find_p = find;
 	travel_p = travel;
}

//ע�ᶨʱ��
int  start_timer(int sec, int usec, struct time_ev* ev)
{
	ELE ele = {sec, usec, ev};
	
	return llist_append(lhandle, &ele);	
}

//ɾ����ʱ��
int  stop_timer(struct time_ev* ev)
{
	llist_delete(lhandle, find_p, ev);
	return 0;
}

//��ⳬʱ
void run_timer(void)
{
	llist_travel(lhandle, travel, NULL);
}
#endif // WIN32

