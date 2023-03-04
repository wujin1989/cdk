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
#include "cdk/cdk-queue.h"
#include "cdk/cdk-mtx.h"
#include "cdk/cdk-cnd.h"
#include "cdk/cdk-thread.h"
#include "cdk/cdk-types.h"
#include <stdlib.h>

static int __thrdpool_thrdfunc(void* arg) {
	
	cdk_thrdpool_t* p = arg;

	cdk_thrdpool_job_t  j;

	while (p->s) {

		cdk_mtx_lock(&p->j_m);
		cdk_thrdpool_job_t* j_p;

		while (p->s && cdk_queue_empty(&p->q)) {
			cdk_cnd_wait(&p->j_c, &p->j_m);
		}
		j_p  = cdk_queue_data(cdk_queue_dequeue(&p->q), cdk_thrdpool_job_t, n);
		j = *j_p;
		cdk_memory_free(j_p);
		cdk_mtx_unlock(&p->j_m);

		if (p->s) { j.fn(j.p); }
	}
	return 0;
}

static void __thrdpool_createthread(cdk_thrdpool_t* p) {

	void*   nt; /* new threads */

	cdk_mtx_lock(&p->t_m);
	nt = realloc(p->t, (p->t_c + 1) * sizeof(cdk_thrd_t));
	if (!nt) { cdk_mtx_unlock(&p->t_m); return; }

	p->t = nt;
	cdk_thrd_create(p->t + p->t_c, __thrdpool_thrdfunc, p);
	

	p->t_c++;
	cdk_mtx_unlock(&p->t_m);
}

cdk_thrdpool_t* cdk_thrdpool_create(void) {

	cdk_thrdpool_t* p = cdk_memory_malloc(sizeof(cdk_thrdpool_t));

	cdk_queue_create(&p->q);
	cdk_mtx_init(&p->t_m);
	cdk_mtx_init(&p->j_m);
	cdk_cnd_init(&p->j_c);
	p->t_c = 0;
	p->s   = true;
	p->t   = NULL;

	for (int i = 0; i < cdk_sysinfo_cpus(); i++) {
		__thrdpool_createthread(p);
	}
	return p;
}

void cdk_thrdpool_destroy(cdk_thrdpool_t* p) {
	
	if (!p) { return; }

	p->s = false;

	cdk_mtx_lock(&p->j_m);
	cdk_cnd_broadcast(&p->j_c);
	cdk_mtx_unlock(&p->j_m);

	for (int i = 0; i < p->t_c; i++) {
		cdk_thrd_join(p->t[i]);
	}
	cdk_mtx_destroy(&p->j_m);
	cdk_mtx_destroy(&p->t_m);
	cdk_cnd_destroy(&p->j_c);

	cdk_memory_free(p->t);
	cdk_memory_free(p);
}

void cdk_thrdpool_post(cdk_thrdpool_t* p, cdk_thrdpool_job_t* j) {

	if (!p || !j) { return; }

	cdk_mtx_lock(&p->j_m);
	cdk_queue_enqueue(&p->q, &j->n);
	cdk_cnd_signal(&p->j_c);
	cdk_mtx_unlock(&p->j_m);
}