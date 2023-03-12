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
#include "cdk/cdk-memory.h"
#include "cdk/cdk-threadpool.h"
#include "cdk/container/cdk-rbtree.h"
#include "cdk/thread/cdk-cnd.h"
#include "cdk/thread/cdk-mtx.h"
#include "cdk/cdk-atomic.h"
#include "cdk/thread/cdk-thread.h"
#include "cdk/cdk-sysinfo.h"
#include "cdk/container/cdk-queue.h"
#include "cdk/cdk-time.h"
#include <stdint.h>

typedef struct cdk_timer_s {
	cdk_thrd_t*  thrds;
	size_t       thrdcnt;
	cdk_rbtree_t rbtree;
	cdk_mtx_t    tmtx;
	cdk_mtx_t    rbmtx;
	cdk_cnd_t    rbcnd;
	bool         status;
}cdk_timer_t;

typedef struct cdk_timer_job_s {
	void  (*routine)(void*);
	void* arg;
	bool  repeat;
	cdk_queue_node_t n;
}cdk_timer_job_t;

typedef struct cdk_timer_jobqueue_s {
	uint64_t          timebase;
	cdk_queue_t       jobs;
	cdk_rbtree_node_t n;
}cdk_timer_jobqueue_t;

static cdk_timer_t timer;
static cdk_atomic_flag_t once_create  = CDK_ATOMIC_FLAG_INIT;
static cdk_atomic_flag_t once_destroy = CDK_ATOMIC_FLAG_INIT;

static void cdk_timer_post(cdk_timer_jobqueue_t* jobs) {

	if (!jobs) {
		return;
	}
	cdk_rbtree_insert(&timer.rbtree, &jobs->n);
	cdk_cnd_signal(&timer.rbcnd);
}

void cdk_timer_add(void (*routine)(void*), void* arg, uint32_t expire, bool repeat) {

	cdk_timer_jobqueue_t* jobs;
	cdk_timer_job_t* job;
	cdk_rbtree_node_key_t key;
	cdk_rbtree_node_t* node;
	uint64_t timebase;

	cdk_mtx_lock(&timer.rbmtx);

	timebase = cdk_time_now();
	key.u64 = timebase + expire;

	job = cdk_memory_malloc(sizeof(cdk_timer_job_t));

	job->routine = routine;
	job->arg = arg;
	job->repeat = repeat;

	node = cdk_rbtree_find(&timer.rbtree, key);

	if (node) {
		jobs = cdk_rbtree_data(node, cdk_timer_jobqueue_t, n);
		cdk_queue_enqueue(&jobs->jobs, &job->n);
	}
	else {
		jobs = cdk_memory_malloc(sizeof(cdk_timer_jobqueue_t));

		jobs->timebase = timebase;
		jobs->n.rb_key = key;
		cdk_queue_create(&jobs->jobs);
		cdk_queue_enqueue(&jobs->jobs, &job->n);
		cdk_timer_post(jobs);
	}
	cdk_mtx_unlock(&timer.rbmtx);
}

static int cdk_timer_thrdfunc(void* arg) {

	while (timer.status) {

		cdk_mtx_lock(&timer.rbmtx);
		cdk_timer_jobqueue_t* jobs;

		while (timer.status && cdk_rbtree_empty(&timer.rbtree)) {
			cdk_cnd_wait(&timer.rbcnd, &timer.rbmtx);
		}
		jobs = cdk_rbtree_data(cdk_rbtree_first(&timer.rbtree), cdk_timer_jobqueue_t, n);

		uint64_t now = cdk_time_now();

		if (jobs->n.rb_key.u64 > now) {

			cdk_mtx_unlock(&timer.rbmtx);
			cdk_time_sleep((uint32_t)(jobs->n.rb_key.u64 - now));
			continue;
		}
		cdk_rbtree_erase(&timer.rbtree, &jobs->n);

		if (timer.status) {

			cdk_timer_job_t* job;
			while (!cdk_queue_empty(&jobs->jobs)) {
				job = cdk_queue_data(cdk_queue_dequeue(&jobs->jobs), cdk_timer_job_t, n);
				job->routine(job->arg);

				if (job->repeat) {
					cdk_timer_add(job->routine, job->arg, (uint32_t)(jobs->n.rb_key.u64 - jobs->timebase), job->repeat);
				}
				cdk_memory_free(job);
			}
		}
		cdk_memory_free(jobs);
		cdk_mtx_unlock(&timer.rbmtx);
	}
	return 0;
}

static void cdk_timer_createthread(void) {

	void* thrds;

	cdk_mtx_lock(&timer.tmtx);
	thrds = realloc(timer.thrds, (timer.thrdcnt + 1) * sizeof(cdk_thrd_t));
	if (!thrds) {
		cdk_mtx_unlock(&timer.tmtx);
		return;
	}
	timer.thrds = thrds;
	cdk_thrd_create(timer.thrds + timer.thrdcnt, cdk_timer_thrdfunc, NULL);

	timer.thrdcnt++;
	cdk_mtx_unlock(&timer.tmtx);
}

void cdk_timer_create(int nthrds) {

	if (cdk_atomic_flag_test_and_set(&once_create)) {
		return;
	}
	cdk_rbtree_create(&timer.rbtree, RB_KEYTYPE_UINT64);

	cdk_mtx_init(&timer.tmtx);
	cdk_mtx_init(&timer.rbmtx);
	cdk_cnd_init(&timer.rbcnd);

	timer.thrdcnt = 0;
	timer.status = true;
	timer.thrds = NULL;

	for (int i = 0; i < nthrds; i++) {
		cdk_timer_createthread();
	}
}

void cdk_timer_destroy(void) {

	if (cdk_atomic_flag_test_and_set(&once_destroy)) {
		return;
	}
	timer.status = false;

	cdk_mtx_lock(&timer.rbmtx);
	cdk_cnd_broadcast(&timer.rbcnd);
	cdk_mtx_unlock(&timer.rbmtx);

	for (int i = 0; i < timer.thrdcnt; i++) {
		cdk_thrd_join(timer.thrds[i]);
	}
	cdk_mtx_destroy(&timer.rbmtx);
	cdk_mtx_destroy(&timer.tmtx);
	cdk_cnd_destroy(&timer.rbcnd);

	cdk_memory_free(timer.thrds);
}

