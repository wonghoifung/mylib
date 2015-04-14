#ifndef SLOWTIMER_HEADER
#define SLOWTIMER_HEADER

#define __KERNEL__
#include <stddef.h>
#include <sys/times.h>
#include "list.h"
#include "log.h"

typedef struct timer_base timer_base_t;

typedef struct stimer {
	struct list_head list;
	unsigned long expires;
	void* ctx;
	void (*function) (void*);
    timer_base_t* base;
} stimer_t;

static void stimer_set(stimer_t* timer, unsigned long expires, void (*fn)(void*), void* ctx);
static int  stimer_add(stimer_t* timer);
static int  stimer_del(stimer_t* timer);
static void stimer_mod(stimer_t* timer, unsigned long expires);

static timer_base_t* stimer_init();
static void stimer_collect();
static void stimer_cleanup(timer_base_t*);

static void stimer_base_set(stimer_t* timer, timer_base_t*);
static void stimer_base_collect(timer_base_t*);

static inline int stimer_valid(const stimer_t * timer)
{
	return timer->list.next != NULL;
}
#define stimer_pending stimer_valid 

#endif 
