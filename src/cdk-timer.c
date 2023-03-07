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
#include "cdk/cdk-queue.h"
#include "cdk/cdk-time.h"


void cdk_timer_create(cdk_timer_t* timer, uint32_t workers) {

	cdk_thrdpool_timed_create(&timer->pool, workers);
}

void cdk_timer_destroy(cdk_timer_t* timer) {

	cdk_thrdpool_timed_destroy(&timer->pool);
}

void cdk_timer_add(cdk_timer_t* timer, void (*routine)(void*), void* arg, uint32_t expire, bool repeat) {
	
	cdk_thrdpool_timed_jobs_t* jobs;
	cdk_thrdpool_timed_job_t* job;
	cdk_rbtree_node_key_t key;
	cdk_rbtree_node_t* node;
	uint64_t timebase;

	timebase = cdk_time_now();
	key.u64  = timebase + expire;

	job = cdk_memory_malloc(sizeof(cdk_thrdpool_timed_job_t));

	job->routine = routine;
	job->arg     = arg;
	job->repeat  = repeat;

	node = cdk_rbtree_find(&timer->pool.rbtree, key);
	if (node) {
		jobs = cdk_rbtree_data(node, cdk_thrdpool_timed_jobs_t, n);
		cdk_queue_enqueue(&jobs->jobs, &job->n);
	}
	else {
		jobs = cdk_memory_malloc(sizeof(cdk_thrdpool_timed_jobs_t));
		
		jobs->timebase = timebase;
		jobs->n.rb_key = key;
		cdk_queue_create(&jobs->jobs);
		cdk_queue_enqueue(&jobs->jobs, &job->n);
		cdk_thrdpool_timed_post(&timer->pool, jobs);
	}
}
