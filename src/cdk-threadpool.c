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

#include "cdk/cdk-sysinfo.h"
#include "cdk/cdk-memory.h"
#include "cdk/cdk-atomic.h"
#include "cdk/container/cdk-queue.h"
#include "cdk/thread/cdk-mtx.h"
#include "cdk/thread/cdk-cnd.h"
#include "cdk/thread/cdk-thread.h"
#include "cdk/cdk-types.h"
#include <stdlib.h>

static int cdk_thrdpool_thrdfunc(void* arg) {
	
	cdk_thrdpool_t* pool = arg;

	while (pool->status) {

		cdk_mtx_lock(&pool->qmtx);
		cdk_thrdpool_job_t* job;

		while (pool->status && cdk_queue_empty(&pool->queue)) {
			cdk_cnd_wait(&pool->qcnd, &pool->qmtx);
		}
		job = cdk_queue_data(cdk_queue_dequeue(&pool->queue), cdk_thrdpool_job_t, n);

		if (pool->status) {
			job->routine(job->arg);
		}
		cdk_memory_free(job);
		cdk_mtx_unlock(&pool->qmtx);
	}
	return 0;
}

static void cdk_thrdpool_createthread(cdk_thrdpool_t* pool) {

	void* thrds;

	cdk_mtx_lock(&pool->tmtx);
	thrds = realloc(pool->thrds, (pool->thrdcnt + 1) * sizeof(cdk_thrd_t));
	if (!thrds) {
		cdk_mtx_unlock(&pool->tmtx);
		return;
	}
	pool->thrds = thrds;
	cdk_thrd_create(pool->thrds + pool->thrdcnt, cdk_thrdpool_thrdfunc, pool);

	pool->thrdcnt++;
	cdk_mtx_unlock(&pool->tmtx);
}

cdk_thrdpool_t* cdk_thrdpool_create(int nthrds) {

	cdk_thrdpool_t* pool = cdk_memory_malloc(sizeof(cdk_thrdpool_t));

	cdk_queue_create(&pool->queue);

	cdk_mtx_init(&pool->tmtx);
	cdk_mtx_init(&pool->qmtx);
	cdk_cnd_init(&pool->qcnd);

	pool->thrdcnt = 0;
	pool->status  = true;
	pool->thrds   = NULL;

	for (int i = 0; i < nthrds; i++) {
		cdk_thrdpool_createthread(pool);
	}
	return pool;
}

void cdk_thrdpool_destroy(cdk_thrdpool_t* pool) {
	
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
}

void cdk_thrdpool_post(cdk_thrdpool_t* pool, cdk_thrdpool_job_t* job) {

	if (!job) {
		return;
	}
	cdk_mtx_lock(&pool->qmtx);
	cdk_queue_enqueue(&pool->queue, &job->n);
	cdk_cnd_signal(&pool->qcnd);
	cdk_mtx_unlock(&pool->qmtx);
}

