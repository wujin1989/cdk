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

#include "cdk/cdk-utils.h"
#include "cdk/container/cdk-queue.h"
#include "cdk/cdk-types.h"
#include <stdlib.h>

static int cdk_thrdpool_thrdfunc(void* arg) {
	
	cdk_thrdpool_t* pool = arg;

	while (pool->status) {

		mtx_lock(&pool->qmtx);
		cdk_thrdpool_job_t* job;

		while (pool->status && cdk_queue_empty(&pool->queue)) {
			cnd_wait(&pool->qcnd, &pool->qmtx);
		}
		job = cdk_queue_data(cdk_queue_dequeue(&pool->queue), cdk_thrdpool_job_t, n);

		if (pool->status) {
			job->routine(job->arg);
		}
		free(job);
		job = NULL;
		mtx_unlock(&pool->qmtx);
	}
	return 0;
}

static void cdk_thrdpool_createthread(cdk_thrdpool_t* pool) {

	void* thrds;

	mtx_lock(&pool->tmtx);
	thrds = realloc(pool->thrds, (pool->thrdcnt + 1) * sizeof(thrd_t));
	if (!thrds) {
		mtx_unlock(&pool->tmtx);
		return;
	}
	pool->thrds = thrds;
	thrd_create(pool->thrds + pool->thrdcnt, cdk_thrdpool_thrdfunc, pool);

	pool->thrdcnt++;
	mtx_unlock(&pool->tmtx);
}

void cdk_thrdpool_create(cdk_thrdpool_t* pool, int nthrds) {

	if (pool) {
		cdk_queue_init(&pool->queue);

		mtx_init(&pool->tmtx, mtx_plain);
		mtx_init(&pool->qmtx, mtx_plain);
		cnd_init(&pool->qcnd);

		pool->thrdcnt = 0;
		pool->status = true;
		pool->thrds = NULL;

		for (int i = 0; i < nthrds; i++) {
			cdk_thrdpool_createthread(pool);
		}
	}
}

void cdk_thrdpool_destroy(cdk_thrdpool_t* pool) {
	
	pool->status = false;

	mtx_lock(&pool->qmtx);
	cnd_broadcast(&pool->qcnd);
	mtx_unlock(&pool->qmtx);

	for (int i = 0; i < pool->thrdcnt; i++) {
		thrd_join(pool->thrds[i], NULL);
	}
	mtx_destroy(&pool->qmtx);
	mtx_destroy(&pool->tmtx);
	cnd_destroy(&pool->qcnd);

	free(pool->thrds);
	pool->thrds = NULL;
}

void cdk_thrdpool_post(cdk_thrdpool_t* pool, cdk_thrdpool_job_t* job) {

	if (!job) {
		return;
	}
	mtx_lock(&pool->qmtx);
	cdk_queue_enqueue(&pool->queue, &job->n);
	cnd_signal(&pool->qcnd);
	mtx_unlock(&pool->qmtx);
}

