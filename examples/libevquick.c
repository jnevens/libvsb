/*************************************************************
 *          Libevquick - event wrapper library
 *   (c) 2012 Daniele Lacamera <root@danielinux.net>
 *              Released under the terms of
 *            GNU Lesser Public License (LGPL)
 *              Version 2.1, February 1999
 *
 *             see COPYING for more details
 */
#include <sys/poll.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include "heap.h"
#include "libevquick.h"
#include <fcntl.h>
#include <assert.h>

static __thread int giveup;


struct evquick_timer_instance
{
	unsigned long long expire;
	struct evquick_timer *ev_timer;
};
typedef struct evquick_timer_instance evquick_timer_instance;

DECLARE_HEAP(evquick_timer_instance, expire);

struct evquick_ctx
{
	int time_machine[2];
	int changed;
	int n_events;
	int last_served;
	struct pollfd *pfd;
	struct evquick_event *events;
	struct evquick_event *_array;
	heap_evquick_timer_instance *timers;
};

static __thread struct evquick_ctx *ctx;

void give_me_a_break(int signo)
{
	char c = 't';
	if (signo == SIGALRM)
		if (write(ctx->time_machine[1], &c, 1) < 0)
			ualarm(1000, 0);
}

#define LOOP_BREAK() give_me_a_break(SIGALRM)

evquick_event *evquick_addevent(int fd, short events,
	void (*callback)(int fd, short revents, void *arg),
	void (*err_callback)(int fd, short revents, void *arg),
	void *arg)
{
	evquick_event *e = malloc(sizeof(evquick_event));
	if (!e)
		return e;
	e->fd = fd;
	e->events = events;
	e->callback = callback;
	e->err_callback = err_callback;
	e->arg = arg;

	ctx->changed = 1;

	e->next = ctx->events;
	ctx->events = e;
	ctx->n_events++;
	LOOP_BREAK();
	return e;
}

void evquick_delevent(evquick_event *e)
{
	evquick_event *cur, *prev;
	ctx->changed = 1;
	cur = ctx->events;
	prev = NULL;
	while(cur) {
		if (cur == e) {
			if (!prev)
				ctx->events = e->next;
			 else
				prev->next = e->next;
			free(e);
		}
		prev = cur;
		cur = cur->next;
	}
	ctx->n_events--;
	LOOP_BREAK();
}

static void timer_trigger(evquick_timer *t, unsigned long long now,
	unsigned long long expire)
{
	evquick_timer_instance tev, *first;
	tev.ev_timer = t;
	tev.expire = expire;
	heap_insert(ctx->timers, &tev);
	first = heap_first(ctx->timers);
	if (first) {
		unsigned long long interval;
		if (now >= first->expire) {
			ualarm(1000, 0);
			return;
		}
		interval = first->expire - now;
		if (interval >= 1000)
			alarm((unsigned)(interval / 1000));
		else
			ualarm((useconds_t)(1000 * (first->expire - now)), 0);
	}
}

static unsigned long long gettimeofdayms(void)
{
	struct timeval tv;
	unsigned long long ret;
	gettimeofday(&tv, NULL);
	ret = (unsigned long long)tv.tv_sec * 1000ULL;
	ret += (unsigned long long)tv.tv_usec / 1000ULL;
	return ret;
}


evquick_timer *evquick_addtimer(
	unsigned long long interval, short flags,
	void (*callback)(void *arg),
	void *arg)
{
	unsigned long long now = gettimeofdayms();

	evquick_timer *t = malloc(sizeof(evquick_timer));
	if (!t)
		return t;
	t->interval = interval;
	t->flags = flags;
	t->callback = callback;
	t->arg = arg;
	timer_trigger(t, now, now + t->interval);
	ctx->changed = 1;

	return t;
}


void evquick_deltimer(evquick_timer *t)
{
	t->flags |= (short)EVQUICK_EV_DISABLED;
	ctx->changed = 1;
}

int evquick_init(void)
{
	int yes = 1;
	ctx = calloc(1, sizeof(struct evquick_ctx));
	if (!ctx)
		return -1;
	ctx->timers = heap_init();
	if (!ctx->timers)
		return -1;
	if(pipe(ctx->time_machine) < 0)
		return -1;
	fcntl(ctx->time_machine[1], O_NONBLOCK, &yes);
	ctx->n_events = 1;
	ctx->changed = 1;
	signal(SIGALRM, give_me_a_break);
	return 0;
}

static void rebuild_poll(void)
{
	int i = 1;
	evquick_event *e = ctx->events;
	void *ptr = NULL;

	if (ctx->pfd) {
		ptr = ctx->pfd;
		ctx->pfd = NULL;
		free(ptr);
	}
	if (ctx->_array) {
		ptr = ctx->_array;
		ctx->_array = NULL;
		free(ptr);
	}
	ctx->pfd = malloc(sizeof(struct pollfd) * (long unsigned)(ctx->n_events));
	ctx->_array = malloc(sizeof(evquick_event) * (long unsigned)(ctx->n_events));

	if ((!ctx->pfd) || (!ctx->_array)) {
		/* TODO: notify error, events are disabled.
		 * perhaps provide a context-wide callback for errors.
		 */
		perror("MEMORY");
		ctx->n_events = 1;
		ctx->changed = 0;
		return;
	}

	ctx->pfd[0].fd = ctx->time_machine[0];
	ctx->pfd[0].events = POLLIN;

	while(e) {
		memcpy(ctx->_array + i, e, sizeof(evquick_event));
		ctx->pfd[i].fd = e->fd;
		ctx->pfd[i++].events = (short)((e->events & (POLLIN | POLLOUT)) | (POLLHUP | POLLERR));
		e = e->next;
	}
	ctx->last_served = 1;
	ctx->changed = 0;
}

static void serve_event(int n)
{
	evquick_event *e = ctx->_array + n;
	if (n >= ctx->n_events)
		return;
	if (e) {
		ctx->last_served = n;
		if ((ctx->pfd[n].revents & (POLLHUP | POLLERR)) && e->err_callback)
			e->err_callback(e->fd, ctx->pfd[n].revents, e->arg);
		else {
			e->callback(e->fd, ctx->pfd[n].revents, e->arg);
		}
	}
}


void timer_check(void)
{
	evquick_timer_instance t, *first;
	unsigned long long now = gettimeofdayms();
	first = heap_first(ctx->timers);
	while(first && (first->expire <= now)) {
		heap_peek(ctx->timers, &t);
		if (!t.ev_timer) {
			first = heap_first(ctx->timers);
			continue;
		}
		if (t.ev_timer->flags & EVQUICK_EV_DISABLED) {
			/* Timer was disabled in the meanwhile.
			 * Take no action, and destroy it.
			 */
			free(t.ev_timer);
		} else if (t.ev_timer->flags & EVQUICK_EV_RETRIGGER) {
			timer_trigger(t.ev_timer, now, now + t.ev_timer->interval);
			t.ev_timer->callback(t.ev_timer->arg);
			/* Don't free the timer, reuse for next instance
			 * that has just been scheduled.
			 */
		} else {
			/* One shot, invoke callback,
			 * then destroy the timer. */
			t.ev_timer->callback(t.ev_timer->arg);
			free(t.ev_timer);
		}
		first = heap_first(ctx->timers);
	}
	if(first) {
		unsigned long long interval = first->expire - now;
		if (interval >= 1000)
			alarm((unsigned)(interval / 1000));
		else
			ualarm((useconds_t) (1000 * (first->expire - now)), 0);
	}
}

void evquick_loop(void)
{
	int pollret, i;
	for(;;) {
		if (giveup)
			break;

		if (ctx->changed) {
			rebuild_poll();
			continue;
		}

		if (ctx->pfd == NULL) {
			sleep(3600);
			ctx->changed = 1;
			continue;
		}


		pollret = poll(ctx->pfd, (unsigned)ctx->n_events, 3600 * 1000);
		if (pollret <= 0)
			continue;

		if ((ctx->pfd[0].revents & POLLIN) == POLLIN) {
			char discard;
			if(read(ctx->time_machine[0], &discard, 1) < 0)
				printf("libevquick: problem with read!\n");
			timer_check();
			continue;
		}
		if (ctx->n_events < 2)
			continue;

		for (i = ctx->last_served +1; i < ctx->n_events; i++) {
			if (ctx->pfd[i].revents != 0) {
				serve_event(i);
				goto end_loop;
			}
		}
		for (i = 1; i <= ctx->last_served; i++) {
			if (ctx->pfd[i].revents != 0) {
				serve_event(i);
				goto end_loop;
			}
		}
	end_loop:
		continue;

	} /* main loop */
}

void evquick_fini(void)
{
	ualarm(1000, 0);
	giveup = 1;
}
