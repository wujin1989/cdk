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

#if defined(__linux__)
#define _DEFAULT_SOURCE
#endif

#include "cdk/cdk-memory.h"
#include "cdk/cdk-types.h"
#include <stdbool.h>
#include <stdlib.h>

#if defined(__linux__)
#include <unistd.h>
#include <sys/syscall.h>
#endif

typedef struct platform_thrd_ctx_s {
	int     (*entry)(void* arg);
	void*   arg;
}platform_thrd_ctx_t;

static void* platform_thrd_start(void* arg) {

	platform_thrd_ctx_t* ctxp;
	platform_thrd_ctx_t  ctx;

	ctxp = arg;
	ctx  = *ctxp;

	cdk_memory_free(ctxp);
	ctx.entry(ctx.arg);

	return NULL;
}

#if defined(__APPLE__)
cdk_tid_t platform_thrd_gettid(void) {

	uint64_t tid;
	pthread_threadid_np(NULL, &tid);

	return tid;
}
#endif

#if defined(__linux__)
cdk_tid_t platform_thrd_gettid(void) {

	return syscall(SYS_gettid);
}
#endif

void platform_thrd_create(cdk_thrd_t* t, int (*h)(void*), void* restrict p) {

	platform_thrd_ctx_t*  ctx;

	ctx = cdk_memory_malloc(sizeof(platform_thrd_ctx_t));

	ctx->entry = h;
	ctx->arg   = p;

	if (pthread_create(&t->tid, NULL, platform_thrd_start, ctx)) {

		cdk_memory_free(ctx);
		abort();
	}
}

void platform_thrd_join(cdk_thrd_t t) {

	if (pthread_join(t.tid, NULL)) {
		abort();
	}
}

void platform_thrd_detach(cdk_thrd_t t) {

	if (pthread_detach(t.tid)) {
		abort();
	}
}

void platform_thrd_once(cdk_once_t* f, void (*h)(void)) {
	
	if (pthread_once(f, h)) {
		abort();
	}
}

bool platform_thrd_equal(cdk_thrd_t t1, cdk_thrd_t t2) {

	if (pthread_equal(t1.tid, t2.tid)) {
		return true;
	}
	return false;
}

cdk_thrd_t platform_thrd_current(void) {

	cdk_thrd_t t;
	t.tid = pthread_self();

	return t;
}