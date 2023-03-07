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

#include "cdk/cdk-threadpool.h"
#include "cdk/cdk-sysinfo.h"
#include "cdk/cdk-memory.h"
#include "cdk/cdk-rbtree.h"
#include "cdk/cdk-queue.h"
#include "cdk/cdk-mtx.h"
#include "cdk/cdk-cnd.h"
#include "cdk/cdk-thread.h"
#include "cdk/cdk-types.h"
#include "cdk/cdk-time.h"
#include <stdlib.h>

static int __thrdpool_thrdfunc(void* arg) {
	
	cdk_thrdpool_t* pool = arg;

	cdk_thrdpool_job_t job;

	while (pool->status) {

		cdk_mtx_lock(&pool->qmtx);
		cdk_thrdpool_job_t* ptr;

		while (pool->status && cdk_queue_empty(&pool->queue)) {
			cdk_cnd_wait(&pool->qcnd, &pool->qmtx);
		}
		ptr = cdk_queue_data(cdk_queue_dequeue(&pool->queue), cdk_thrdpool_job_t, n);
		job = *ptr;
		cdk_memory_free(ptr);
		cdk_mtx_unlock(&pool->qmtx);

		if (pool->status) { 
			job.routine(job.arg);
		}
	}
	return 0;
}

static void __thrdpool_createthread(cdk_thrdpool_t* pool) {

	void* thrds;

	cdk_mtx_lock(&pool->tmtx);
	thrds = realloc(pool->thrds, (pool->thrdcnt + 1) * sizeof(cdk_thrd_t));
	if (!thrds) {
		cdk_mtx_unlock(&pool->tmtx);
		return;
	}
	pool->thrds = thrds;
	cdk_thrd_create(pool->thrds + pool->thrdcnt, __thrdpool_thrdfunc, pool);

	pool->thrdcnt++;
	cdk_mtx_unlock(&pool->tmtx);
}

static int __thrdpool_timed_thrdfunc(void* arg) {

	cdk_thrdpool_timed_t* pool = arg;

	while (pool->status) {

		cdk_mtx_lock(&pool->rbmtx);
		cdk_thrdpool_timed_jobs_t* jobs;

		while (pool->status && cdk_rbtree_empty(&pool->rbtree)) {
			cdk_cnd_wait(&pool->rbcnd, &pool->rbmtx);
		}
		jobs = cdk_rbtree_data(cdk_rbtree_first(&pool->rbtree), cdk_thrdpool_timed_jobs_t, n);

		uint64_t now = cdk_time_now();

		if (jobs->n.rb_key.u64 > now) {

			cdk_mtx_unlock(&pool->rbmtx);
			cdk_time_sleep((uint32_t)(jobs->n.rb_key.u64 - now));
			continue;
		}
		cdk_rbtree_erase(&pool->rbtree, &jobs->n);
		cdk_mtx_unlock(&pool->rbmtx);

		if (pool->status) {

			cdk_thrdpool_timed_job_t* job;
			while (!cdk_queue_empty(&jobs->jobs)) {
				job = cdk_queue_data(cdk_queue_dequeue(&jobs->jobs), cdk_thrdpool_timed_job_t, n);
				job->routine(job->arg);

				if (job->repeat) {
					cdk_thrdpool_timed_job_t* newjob;
					cdk_thrdpool_timed_jobs_t* newjobs;

					newjob = cdk_memory_malloc(sizeof(cdk_thrdpool_timed_job_t));
					newjob->routine = job->routine;
					newjob->arg = job->arg;
					newjob->repeat = job->repeat;

					newjobs = cdk_memory_malloc(sizeof(cdk_thrdpool_timed_jobs_t));

					newjobs->timebase = cdk_time_now();
					newjobs->n.rb_key.u64 = (jobs->n.rb_key.u64 - jobs->timebase) + newjobs->timebase;
					cdk_queue_create(&newjobs->jobs);
					cdk_queue_enqueue(&newjobs->jobs, &newjob->n);
					
					cdk_thrdpool_timed_post(pool, newjobs);
				}
				cdk_memory_free(job);
			}
		}
		cdk_memory_free(jobs);
	}
	return 0;
}

static void __thrdpool_timed_createthread(cdk_thrdpool_timed_t* pool) {

	void* thrds;

	cdk_mtx_lock(&pool->tmtx);
	thrds = realloc(pool->thrds, (pool->thrdcnt + 1) * sizeof(cdk_thrd_t));
	if (!thrds) {
		cdk_mtx_unlock(&pool->tmtx);
		return;
	}
	pool->thrds = thrds;
	cdk_thrd_create(pool->thrds + pool->thrdcnt, __thrdpool_timed_thrdfunc, pool);

	pool->thrdcnt++;
	cdk_mtx_unlock(&pool->tmtx);
}

void cdk_thrdpool_create(cdk_thrdpool_t* pool, int workers) {

	cdk_queue_create(&pool->queue);

	cdk_mtx_init(&pool->tmtx);
	cdk_mtx_init(&pool->qmtx);
	cdk_cnd_init(&pool->qcnd);

	pool->thrdcnt = 0;
	pool->status  = true;
	pool->thrds   = NULL;

	if (!workers) {
		workers = cdk_sysinfo_cpus();
	}
	for (int i = 0; i < workers; i++) {
		__thrdpool_createthread(pool);
	}
}

void cdk_thrdpool_timed_create(cdk_thrdpool_timed_t* pool, int workers) {

	cdk_rbtree_create(&pool->rbtree, RB_KEYTYPE_UINT64);

	cdk_mtx_init(&pool->tmtx);
	cdk_mtx_init(&pool->rbmtx);
	cdk_cnd_init(&pool->rbcnd);

	pool->thrdcnt = 0;
	pool->status = true;
	pool->thrds = NULL;

	if (!workers) {
		workers = cdk_sysinfo_cpus();
	}
	for (int i = 0; i < workers; i++) {
		__thrdpool_timed_createthread(pool);
	}
}

void cdk_thrdpool_destroy(cdk_thrdpool_t* pool) {
	
	if (!pool) {
		return;
	}
	pool->status = false;

	cdk_mtx_lock(&pool->qmtx);
	cdk_cnd_broadcast(&pool->qcnd);
	cdk_mtx_unlock(&pool->qmtx);

	for (int i = 0; i < pool->thrdcnt; i++) {
		cdk_thrd_join(pool->thrds[i]);
	}
	cdk_mtx_destroy(&pool->qmtx);
	cdk_mtx_destroy(&pool->tmtx);
	cdk_cnd_destroy(&pool->qcnd);

	cdk_memory_free(pool->thrds);
	cdk_memory_free(pool);
}

void cdk_thrdpool_timed_destroy(cdk_thrdpool_timed_t* pool) {

	if (!pool) {
		return;
	}
	pool->status = false;

	cdk_mtx_lock(&pool->rbmtx);
	cdk_cnd_broadcast(&pool->rbcnd);
	cdk_mtx_unlock(&pool->rbmtx);

	for (int i = 0; i < pool->thrdcnt; i++) {
		cdk_thrd_join(pool->thrds[i]);
	}
	cdk_mtx_destroy(&pool->rbmtx);
	cdk_mtx_destroy(&pool->tmtx);
	cdk_cnd_destroy(&pool->rbcnd);

	cdk_memory_free(pool->thrds);
	cdk_memory_free(pool);
}

void cdk_thrdpool_post(cdk_thrdpool_t* pool, cdk_thrdpool_job_t* job) {

	if (!pool || !job) {
		return;
	}
	cdk_mtx_lock(&pool->qmtx);
	cdk_queue_enqueue(&pool->queue, &job->n);
	cdk_cnd_signal(&pool->qcnd);
	cdk_mtx_unlock(&pool->qmtx);
}

void cdk_thrdpool_timed_post(cdk_thrdpool_timed_t* pool, cdk_thrdpool_timed_jobs_t* jobs) {

	if (!pool || !jobs) {
		return;
	}
	cdk_mtx_lock(&pool->rbmtx);
	cdk_rbtree_insert(&pool->rbtree, &jobs->n);
	cdk_cnd_signal(&pool->rbcnd);
	cdk_mtx_unlock(&pool->rbmtx);
}