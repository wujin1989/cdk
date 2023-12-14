/** Copyright (c), Wu Jin <wujin.developer@gmail.com>
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

#include "cdk/cdk-types.h"
#include "cdk/cdk-threadpool.h"
#include "cdk/container/cdk-rbtree.h"
#include "cdk/cdk-utils.h"
#include "cdk/container/cdk-queue.h"
#include "cdk/cdk-time.h"
#include <stdint.h>
#include <stdatomic.h>
#include <stdlib.h>

typedef struct timer_entry_s {
	cdk_list_t        joblst;
	cdk_rbtree_node_t n;
}timer_entry_t;

static inline void _timer_add(cdk_timer_t* timer, timer_entry_t* entry) {
	if (!entry) {
		return;
	}
	cdk_rbtree_insert(&timer->rbtree, &entry->n);
	cnd_signal(&timer->rbcnd);
}

cdk_timer_job_t* cdk_timer_add(cdk_timer_t* timer, void (*routine)(void*), void* arg, uint32_t expire, bool repeat) {
	timer_entry_t* entry;
	cdk_rbtree_node_key_t key;
	cdk_rbtree_node_t* node;
	cdk_timer_job_t* job;
	
	job = malloc(sizeof(cdk_timer_job_t));
	if (job) {
		job->routine = routine;
		job->arg = arg;
		job->expire = expire;
		job->repeat = repeat;
		job->birthtime = cdk_time_now();

		mtx_lock(&timer->rbmtx);
		key.u64 = job->birthtime + job->expire;
		node = cdk_rbtree_find(&timer->rbtree, key);
		if (node) {
			entry = cdk_rbtree_data(node, timer_entry_t, n);
			cdk_list_insert_tail(&entry->joblst, &job->n);
		} else {
			entry = malloc(sizeof(timer_entry_t));
			if (entry) {
				entry->n.rb_key = key;
				cdk_list_init(&entry->joblst);
				cdk_list_insert_tail(&entry->joblst, &job->n);
				_timer_add(timer, entry);
			}
		}
		mtx_unlock(&timer->rbmtx);
	}
	return job;
}

void cdk_timer_del(cdk_timer_t* timer, cdk_timer_job_t* job) {
	timer_entry_t* entry;
	cdk_rbtree_node_key_t key;
	cdk_rbtree_node_t* node;

	mtx_lock(&timer->rbmtx);
	key.u64 = job->birthtime + job->expire;
	node = cdk_rbtree_find(&timer->rbtree, key);
	if (node) {
		entry = cdk_rbtree_data(node, timer_entry_t, n);
		if (!cdk_list_empty(&entry->joblst)) {
			cdk_list_remove(&job->n);
			free(job);
			job = NULL;
		} else {
			cdk_rbtree_erase(&timer->rbtree, &entry->n);
			free(entry);
			entry = NULL;
		}
	}
	mtx_unlock(&timer->rbmtx);
}

static inline int _timer_thrdfunc(void* arg) {
	cdk_timer_t* timer = arg;
	while (timer->status) {
		mtx_lock(&timer->rbmtx);
		timer_entry_t* entry;
		while (timer->status && cdk_rbtree_empty(&timer->rbtree)) {
			cnd_wait(&timer->rbcnd, &timer->rbmtx);
		}
		entry = cdk_rbtree_data(cdk_rbtree_first(&timer->rbtree), timer_entry_t, n);
		uint64_t now = cdk_time_now();
		if (entry->n.rb_key.u64 > now) {
			mtx_unlock(&timer->rbmtx);
			cdk_time_sleep((uint32_t)(entry->n.rb_key.u64 - now));
			continue;
		}
		cdk_rbtree_erase(&timer->rbtree, &entry->n);
		mtx_unlock(&timer->rbmtx);

		cdk_timer_job_t* job;
		while (!cdk_list_empty(&entry->joblst)) {
			job = cdk_list_data(cdk_list_head(&entry->joblst), cdk_timer_job_t, n);
			cdk_list_remove(&job->n);
			if (timer->status) {
				job->routine(job->arg);
				if (job->repeat) {
					cdk_timer_add(timer, job->routine, job->arg, job->expire, job->repeat);
					continue;
				}
			}
			free(job);
			job = NULL;
		}
		free(entry);
		entry = NULL;
	}
	return 0;
}

static inline void _timer_createthread(cdk_timer_t* timer) {
	void* thrds;
	mtx_lock(&timer->tmtx);
	thrds = realloc(timer->thrds, (timer->thrdcnt + 1) * sizeof(thrd_t));
	if (!thrds) {
		mtx_unlock(&timer->tmtx);
		return;
	}
	timer->thrds = thrds;
	thrd_create(timer->thrds + timer->thrdcnt, _timer_thrdfunc, timer);

	timer->thrdcnt++;
	mtx_unlock(&timer->tmtx);
}

void cdk_timer_create(cdk_timer_t* timer, int nthrds) {
	cdk_rbtree_init(&timer->rbtree, default_keycmp_u64);

	mtx_init(&timer->tmtx, mtx_plain);
	mtx_init(&timer->rbmtx, mtx_plain);
	cnd_init(&timer->rbcnd);

	timer->thrdcnt = 0;
	timer->status = true;
	timer->thrds = NULL;
	for (int i = 0; i < nthrds; i++) {
		_timer_createthread(timer);
	}
}

void cdk_timer_destroy(cdk_timer_t* timer) {
	timer->status = false;
	mtx_lock(&timer->rbmtx);
	cnd_broadcast(&timer->rbcnd);
	mtx_unlock(&timer->rbmtx);

	for (int i = 0; i < timer->thrdcnt; i++) {
		thrd_join(timer->thrds[i], NULL);
	}
	mtx_destroy(&timer->rbmtx);
	mtx_destroy(&timer->tmtx);
	cnd_destroy(&timer->rbcnd);

	free(timer->thrds);
	timer->thrds = NULL;
}

