/*************************************************************
 *          Libevquick - event wrapper library
 *   (c) 2012 Daniele Lacamera <root@danielinux.net>
 *              Released under the terms of
 *            GNU Lesser Public License (LGPL)
 *              Version 2.1, February 1999
 *
 *             see COPYING for more details
 */
#ifndef __LIBEVQUICK
#define __LIBEVQUICK
#include <sys/poll.h>


#define EVQUICK_EV_READ POLLIN
#define EVQUICK_EV_WRITE POLLOUT
#define EVQUICK_EV_NORMAL 0x0000
#define EVQUICK_EV_RETRIGGER 0x4000
#define EVQUICK_EV_DISABLED  0x8000

struct evquick_event
{
	int fd;
	short events;
	void (*callback)(int fd, short revents, void *arg);
	void (*err_callback)(int fd, short revents, void *arg);
	void *arg;
	struct evquick_event *next;
};
typedef struct evquick_event evquick_event;


struct evquick_timer
{
	unsigned long long interval;
	short flags;
	void (*callback)(void *arg);
	void *arg;
};
typedef struct evquick_timer evquick_timer;

evquick_event *evquick_addevent(int fd, short events,
							    void (*callback)
									(int fd, short revents, void *arg),
    							void (*err_callback)
									(int fd, short revents, void *arg),
    							void *arg);

void evquick_delevent(evquick_event *e);

evquick_timer *evquick_addtimer(unsigned long long interval, short flags,
							    void (*callback)(void *arg),
							    void *arg);

void evquick_deltimer(evquick_timer *t);

void evquick_loop(void);
int evquick_init(void);
void evquick_fini(void);

#endif



