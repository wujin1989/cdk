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
#include "cdk/cdk-rbtree.h"
#include "cdk/cdk-cnd.h"
#include "cdk/cdk-mtx.h"
#include "cdk/cdk-thread.h"
#include "cdk/cdk-sysinfo.h"
#include "cdk/cdk-queue.h"
#include "cdk/cdk-time.h"

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

typedef struct cdk_timer_jobs_s {
	uint64_t          timebase;
	cdk_queue_t       jobs;
	cdk_rbtree_node_t n;
}cdk_timer_jobs_t;

static cdk_timer_t __g_timer;

static void __timer_post(cdk_timer_jobs_t* jobs) {

	if (!jobs) {
		return;
	}
	cdk_mtx_lock(&__g_timer.rbmtx);
	cdk_rbtree_insert(&__g_timer.rbtree, &jobs->n);
	cdk_cnd_signal(&__g_timer.rbcnd);
	cdk_mtx_unlock(&__g_timer.rbmtx);
}

static int __timer_thrdfunc(void* arg) {

	while (__g_timer.status) {

		cdk_mtx_lock(&__g_timer.rbmtx);
		cdk_timer_jobs_t* jobs;

		while (__g_timer.status && cdk_rbtree_empty(&__g_timer.rbtree)) {
			cdk_cnd_wait(&__g_timer.rbcnd, &__g_timer.rbmtx);
		}
		jobs = cdk_rbtree_data(cdk_rbtree_first(&__g_timer.rbtree), cdk_timer_jobs_t, n);

		uint64_t now = cdk_time_now();

		if (jobs->n.rb_key.u64 > now) {

			cdk_mtx_unlock(&__g_timer.rbmtx);
			cdk_time_sleep((uint32_t)(jobs->n.rb_key.u64 - now));
			continue;
		}
		cdk_rbtree_erase(&__g_timer.rbtree, &jobs->n);
		cdk_mtx_unlock(&__g_timer.rbmtx);

		if (__g_timer.status) {

			cdk_timer_job_t* job;
			while (!cdk_queue_empty(&jobs->jobs)) {
				job = cdk_queue_data(cdk_queue_dequeue(&jobs->jobs), cdk_timer_job_t, n);
				job->routine(job->arg);

				if (job->repeat) {
					cdk_timer_job_t* newjob;
					cdk_timer_jobs_t* newjobs;

					newjob = cdk_memory_malloc(sizeof(cdk_timer_job_t));
					newjob->routine = job->routine;
					newjob->arg = job->arg;
					newjob->repeat = job->repeat;

					newjobs = cdk_memory_malloc(sizeof(cdk_timer_jobs_t));

					newjobs->timebase = cdk_time_now();
					newjobs->n.rb_key.u64 = (jobs->n.rb_key.u64 - jobs->timebase) + newjobs->timebase;
					cdk_queue_create(&newjobs->jobs);
					cdk_queue_enqueue(&newjobs->jobs, &newjob->n);

					__timer_post(newjobs);
				}
				cdk_memory_free(job);
			}
		}
		cdk_memory_free(jobs);
	}
	return 0;
}

static void __timer_createthread(void) {

	void* thrds;

	cdk_mtx_lock(&__g_timer.tmtx);
	thrds = realloc(__g_timer.thrds, (__g_timer.thrdcnt + 1) * sizeof(cdk_thrd_t));
	if (!thrds) {
		cdk_mtx_unlock(&__g_timer.tmtx);
		return;
	}
	__g_timer.thrds = thrds;
	cdk_thrd_create(__g_timer.thrds + __g_timer.thrdcnt, __timer_thrdfunc, NULL);

	__g_timer.thrdcnt++;
	cdk_mtx_unlock(&__g_timer.tmtx);
}

void cdk_timer_create(uint32_t workers) {

	cdk_rbtree_create(&__g_timer.rbtree, RB_KEYTYPE_UINT64);

	cdk_mtx_init(&__g_timer.tmtx);
	cdk_mtx_init(&__g_timer.rbmtx);
	cdk_cnd_init(&__g_timer.rbcnd);

	__g_timer.thrdcnt = 0;
	__g_timer.status = true;
	__g_timer.thrds = NULL;

	if (!workers) {
		workers = cdk_sysinfo_cpus();
	}
	for (int i = 0; i < workers; i++) {
		__timer_createthread();
	}
}

void cdk_timer_destroy(void) {

	__g_timer.status = false;

	cdk_mtx_lock(&__g_timer.rbmtx);
	cdk_cnd_broadcast(&__g_timer.rbcnd);
	cdk_mtx_unlock(&__g_timer.rbmtx);

	for (int i = 0; i < __g_timer.thrdcnt; i++) {
		cdk_thrd_join(__g_timer.thrds[i]);
	}
	cdk_mtx_destroy(&__g_timer.rbmtx);
	cdk_mtx_destroy(&__g_timer.tmtx);
	cdk_cnd_destroy(&__g_timer.rbcnd);

	cdk_memory_free(__g_timer.thrds);
}

void cdk_timer_add(void (*routine)(void*), void* arg, uint32_t expire, bool repeat) {
	
	cdk_timer_jobs_t* jobs;
	cdk_timer_job_t* job;
	cdk_rbtree_node_key_t key;
	cdk_rbtree_node_t* node;
	uint64_t timebase;

	timebase = cdk_time_now();
	key.u64  = timebase + expire;

	job = cdk_memory_malloc(sizeof(cdk_timer_job_t));

	job->routine = routine;
	job->arg     = arg;
	job->repeat  = repeat;

	node = cdk_rbtree_find(&__g_timer.rbtree, key);
	if (node) {
		jobs = cdk_rbtree_data(node, cdk_timer_jobs_t, n);
		cdk_queue_enqueue(&jobs->jobs, &job->n);
	}
	else {
		jobs = cdk_memory_malloc(sizeof(cdk_timer_jobs_t));
		
		jobs->timebase = timebase;
		jobs->n.rb_key = key;
		cdk_queue_create(&jobs->jobs);
		cdk_queue_enqueue(&jobs->jobs, &job->n);
		__timer_post(jobs);
	}
}

