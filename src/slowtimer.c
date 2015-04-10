/*
 * KTCPVS       An implementation of the TCP Virtual Server daemon inside
 *              kernel for the LINUX operating system. KTCPVS can be used
 *              to build a moderately scalable and highly available server
 *              based on a cluster of servers, with more flexibility.
 *
 * tcp_vs_timer.c: slow timer for collecting stale entries
 *
 * Version:     $Id: tcp_vs_timer.c,v 1.5 2003/05/23 02:51:12 wensong Exp $
 *
 * Authors:     Wensong Zhang <wensong@linuxvirtualserver.org>
 *
 *              This program is free software; you can redistribute it and/or
 *              modify it under the terms of the GNU General Public License
 *              as published by the Free Software Foundation; either version
 *              2 of the License, or (at your option) any later version.
 *
 */

#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include "slowtimer.h"
#include "log.h"

#define spin_lock(x)
#define spin_unlock(x)

#ifndef EXPORT
#define EXPORT static inline
#endif

/*
 * The following block implements slow timers for KTCPVS, most code is stolen
 * from linux/kernel/timer.c.
 */
#define SHIFT_BITS 6
#define TVN_BITS 8
#define TVR_BITS 10
#define TVN_SIZE (1 << TVN_BITS)//2^8
#define TVR_SIZE (1 << TVR_BITS)//1024
#define TVN_MASK (TVN_SIZE - 1)//2^8 - 1
#define TVR_MASK (TVR_SIZE - 1)//1023

struct stimer_vec {
    int index;
    struct list_head vec[TVN_SIZE];
};

struct stimer_vec_root {
    int index;
    struct list_head vec[TVR_SIZE];
};

/*
static unsigned long stimer_jiffies = 0;

static struct stimer_vec sltv3 = { 0 };
static struct stimer_vec sltv2 = { 0 };
static struct stimer_vec_root sltv1 = { 0 };

static struct stimer_vec *const sltvecs[] = {
    (struct stimer_vec *) &sltv1, &sltv2, &sltv3
};
*/

//#define NOOF_SLTVECS (sizeof(sltvecs) / sizeof(sltvecs[0]))
#define NOOF_SLTVECS 3


struct timer_base {
    struct stimer_vec_root sltv1;
    struct stimer_vec sltv2;
    struct stimer_vec sltv3;
    struct stimer_vec* sltvecs[3];
    unsigned long stimer_jiffies;
};

static struct timer_base* t_base = 0;

static inline void
init_stimer_vecs(timer_base_t* base)
{
    int i;

    base->sltv1.index = 0;
    base->sltv2.index = 0;
    base->sltv3.index = 0;
    for (i = 0; i < TVN_SIZE; i++) {
        INIT_LIST_HEAD((struct list_head *) base->sltv3.vec + i);
        INIT_LIST_HEAD(base->sltv2.vec + i);
    }
    for (i = 0; i < TVR_SIZE; i++)
        INIT_LIST_HEAD(base->sltv1.vec + i);

    base->sltvecs[0] = (struct stimer_vec*)&base->sltv1;
    base->sltvecs[1] = &base->sltv2;
    base->sltvecs[2] = &base->sltv3;
	base->stimer_jiffies = 0;
}

static inline void
internal_add_slowtimer(timer_base_t* base, stimer_t * timer)
{
    /*
     * must hold the slowtimer lock when calling this
     */
    unsigned long expires = timer->expires;
    unsigned long idx = expires - base->stimer_jiffies;
	//log_debug("expires:=%d,stimer_jiffies:=%d",expires,base->stimer_jiffies);
    struct list_head *vec;

    if (idx < 1 << (SHIFT_BITS + TVR_BITS)) {
        int i = (expires >> SHIFT_BITS) & TVR_MASK;
        vec = base->sltv1.vec + i;
		//log_debug("time_diff:=%d,insert_index:=%d,sltv1.index:=%d",idx,i,base->sltv1.index);
    }
    else if (idx < 1 << (SHIFT_BITS + TVR_BITS + TVN_BITS)) {
        int i = (expires >> (SHIFT_BITS + TVR_BITS)) & TVN_MASK;
        vec = base->sltv2.vec + i;
    }
    else if ((signed long) idx < 0) {
        /*
         * can happen if you add a timer with expires == jiffies,
         * or you set a timer to go off in the past
         */
        vec = base->sltv1.vec + base->sltv1.index;
    }
    else if (idx <= 0xffffffffUL) {
        int i =
            (expires >> (SHIFT_BITS + TVR_BITS + TVN_BITS)) &
            TVN_MASK;
        vec = base->sltv3.vec + i;
    }
    else {
        /* Can only get here on architectures with 64-bit jiffies */
        INIT_LIST_HEAD(&timer->list);
    }
    /*
     * Timers are FIFO!
     */
    list_add(&timer->list, vec->prev);
}

EXPORT int
stimer_add(stimer_t * timer)
{
    assert(timer && timer->base);
    spin_lock(&tcp_vs_stimer_lock);
    if (timer->list.next)
        goto bug;
    internal_add_slowtimer(timer->base, timer);
    spin_unlock(&tcp_vs_stimer_lock);
    return 0 ;

bug:
    errno = EEXIST;
    return -1;
}

static inline int
detach_slowtimer(stimer_t * timer)
{
    if (!stimer_pending(timer)) {
        errno = ENOENT; //链表中根本就不存在该项
        return -1;
    }
    list_del(&timer->list);
    return 0;
}

EXPORT void
stimer_mod(stimer_t * timer, unsigned long expires)
{
    int ret;

    spin_lock(&tcp_vs_stimer_lock);
    timer->expires = expires;
    ret = detach_slowtimer(timer);
    internal_add_slowtimer(timer->base, timer);
    spin_unlock(&tcp_vs_stimer_lock);
}

EXPORT int
stimer_del(stimer_t * timer)
{
    int ret;

    spin_lock(&tcp_vs_stimer_lock);
    ret = detach_slowtimer(timer);
    timer->list.next = timer->list.prev = 0;
    spin_unlock(&tcp_vs_stimer_lock);
    return ret;
}


static inline void
cascade_slowtimers(struct stimer_vec *tv)
{
    /*
     * cascade all the timers from tv up one level
     */
    struct list_head *head, *curr, *next;

    head = tv->vec + tv->index;
    curr = head->next;

    /*
     * We are removing _all_ timers from the list, so we don't  have to
     * detach them individually, just clear the list afterwards.
     */
    while (curr != head) {
        stimer_t *tmp;

        tmp = list_entry(curr, stimer_t, list);
        next = curr->next;
        list_del(curr); // not needed
        internal_add_slowtimer(tmp->base, tmp);
        curr = next;
    }
    INIT_LIST_HEAD(head);
    tv->index = (tv->index + 1) & TVN_MASK;
}

static inline void
run_stimer_list(timer_base_t* base, unsigned long jiffies)
{
    spin_lock(&tcp_vs_stimer_lock);
    while ((long) (jiffies - base->stimer_jiffies) >= 0) {
        struct list_head *head, *curr;
        if (!base->sltv1.index) {
            int n = 1;
            do {
                cascade_slowtimers(base->sltvecs[n]);
            }
            while (base->sltvecs[n]->index == 1
                    && ++n < NOOF_SLTVECS);
        }
repeat:
        head = base->sltv1.vec + base->sltv1.index;
        curr = head->next;
        if (curr != head) {
            stimer_t *timer;
            void (*fn) (void*);
            void* ctx;

            timer = list_entry(curr, stimer_t, list);
            fn = timer->function;
            ctx = timer->ctx;

            detach_slowtimer(timer);
            timer->list.next = timer->list.prev = NULL;
            spin_unlock(&tcp_vs_stimer_lock);
            fn(ctx);
            spin_lock(&tcp_vs_stimer_lock);
            goto repeat;
        }
        base->stimer_jiffies += 1 << SHIFT_BITS;
		//base->stimer_jiffies += 1;
        base->sltv1.index = (base->sltv1.index + 1) & TVR_MASK;
    }
    spin_unlock(&tcp_vs_stimer_lock);
}


/*
 * The function to collect stale timers must be activated from the
 * outside periodically (such as every second).
 */
EXPORT void stimer_collect()
{
    run_stimer_list(t_base, times(NULL));
}


EXPORT timer_base_t* stimer_init()
{
    timer_base_t* base = malloc(sizeof(timer_base_t));
    if (!base) 
        return 0;
    /* initialize the slowtimer vectors */
    init_stimer_vecs(base);
    t_base = base;
    return base;
}


EXPORT void stimer_cleanup(timer_base_t* base)
{
    if (!base)
        base = t_base;
    free(base);
}

/**
 * 设置定时器
 * @param expires 过期时间，单位：微秒。注意是相对时间
 */
EXPORT void stimer_set(stimer_t* timer, unsigned long expires, void (*fn) (void*), void* ctx)
{
	timer->list.next = timer->list.prev = NULL;
    timer->expires = times(NULL)+ expires/10;//0.01s
    timer->ctx = ctx;
    timer->function = fn;
    timer->base = t_base;
}

EXPORT void stimer_base_set(stimer_t* timer, timer_base_t* base)
{
    timer->base = base;
}

EXPORT void stimer_base_collect(timer_base_t* base)
{
    run_stimer_list(base, times(NULL));
}


