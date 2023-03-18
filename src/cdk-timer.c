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

typedef struct cdk_timer_s {
	thrd_t* thrds;
	size_t thrdcnt;
	cdk_rbtree_t rbtree;
	mtx_t tmtx;
	mtx_t rbmtx;
	cnd_t rbcnd;
	bool status;
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
static atomic_flag once_create  = ATOMIC_FLAG_INIT;
static atomic_flag once_destroy = ATOMIC_FLAG_INIT;

static void cdk_timer_post(cdk_timer_jobqueue_t* jobs) {

	if (!jobs) {
		return;
	}
	cdk_rbtree_insert(&timer.rbtree, &jobs->n);
	cnd_signal(&timer.rbcnd);
}

void cdk_timer_add(void (*routine)(void*), void* arg, uint32_t expire, bool repeat) {

	cdk_timer_jobqueue_t* jobs;
	cdk_timer_job_t* job;
	cdk_rbtree_node_key_t key;
	cdk_rbtree_node_t* node;
	uint64_t timebase;

	mtx_lock(&timer.rbmtx);

	timebase = cdk_time_now();
	key.u64 = timebase + expire;

	job = malloc(sizeof(cdk_timer_job_t));

	job->routine = routine;
	job->arg = arg;
	job->repeat = repeat;

	node = cdk_rbtree_find(&timer.rbtree, key);

	if (node) {
		jobs = cdk_rbtree_data(node, cdk_timer_jobqueue_t, n);
		cdk_queue_enqueue(&jobs->jobs, &job->n);
	}
	else {
		jobs = malloc(sizeof(cdk_timer_jobqueue_t));

		jobs->timebase = timebase;
		jobs->n.rb_key = key;
		cdk_queue_create(&jobs->jobs);
		cdk_queue_enqueue(&jobs->jobs, &job->n);
		cdk_timer_post(jobs);
	}
	mtx_unlock(&timer.rbmtx);
}

static int cdk_timer_thrdfunc(void* arg) {

	while (timer.status) {

		mtx_lock(&timer.rbmtx);
		cdk_timer_jobqueue_t* jobs;

		while (timer.status && cdk_rbtree_empty(&timer.rbtree)) {
			cnd_wait(&timer.rbcnd, &timer.rbmtx);
		}
		jobs = cdk_rbtree_data(cdk_rbtree_first(&timer.rbtree), cdk_timer_jobqueue_t, n);

		uint64_t now = cdk_time_now();

		if (jobs->n.rb_key.u64 > now) {

			mtx_unlock(&timer.rbmtx);
			cdk_time_sleep((uint32_t)(jobs->n.rb_key.u64 - now));
			continue;
		}
		cdk_rbtree_erase(&timer.rbtree, &jobs->n);
		mtx_unlock(&timer.rbmtx);

		cdk_timer_job_t* job;
		while (!cdk_queue_empty(&jobs->jobs)) {
			job = cdk_queue_data(cdk_queue_dequeue(&jobs->jobs), cdk_timer_job_t, n);

			if (timer.status) {
				job->routine(job->arg);
				if (job->repeat) {
					cdk_timer_add(job->routine, job->arg, (uint32_t)(jobs->n.rb_key.u64 - jobs->timebase), job->repeat);
				}
			}
			free(job);
			job = NULL;
		}
		free(jobs);
		jobs = NULL;
	}
	return 0;
}

static void cdk_timer_createthread(void) {

	void* thrds;

	mtx_lock(&timer.tmtx);
	thrds = realloc(timer.thrds, (timer.thrdcnt + 1) * sizeof(thrd_t));
	if (!thrds) {
		mtx_unlock(&timer.tmtx);
		return;
	}
	timer.thrds = thrds;
	thrd_create(timer.thrds + timer.thrdcnt, cdk_timer_thrdfunc, NULL);

	timer.thrdcnt++;
	mtx_unlock(&timer.tmtx);
}

void cdk_timer_create(int nthrds) {

	if (atomic_flag_test_and_set(&once_create)) {
		return;
	}
	cdk_rbtree_create(&timer.rbtree, RB_KEYTYPE_UINT64);

	mtx_init(&timer.tmtx, mtx_plain);
	mtx_init(&timer.rbmtx, mtx_plain);
	cnd_init(&timer.rbcnd);

	timer.thrdcnt = 0;
	timer.status = true;
	timer.thrds = NULL;

	for (int i = 0; i < nthrds; i++) {
		cdk_timer_createthread();
	}
}

void cdk_timer_destroy(void) {

	if (atomic_flag_test_and_set(&once_destroy)) {
		return;
	}
	timer.status = false;

	mtx_lock(&timer.rbmtx);
	cnd_broadcast(&timer.rbcnd);
	mtx_unlock(&timer.rbmtx);

	for (int i = 0; i < timer.thrdcnt; i++) {
		thrd_join(timer.thrds[i], NULL);
	}
	mtx_destroy(&timer.rbmtx);
	mtx_destroy(&timer.tmtx);
	cnd_destroy(&timer.rbcnd);

	free(timer.thrds);
	timer.thrds = NULL;
}

