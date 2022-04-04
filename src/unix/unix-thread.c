/** Copyright (c) 2022, Wu Jin <wujin.developer@gmail.com>
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

#include "unix-thread.h"
#include "cdk/cdk-memory.h"
#include <stdbool.h>
#include <stdlib.h>

#if defined(__linux__)
#include <unistd.h>
#include <sys/syscall.h>
#endif


#if defined(__APPLE__)
uint64_t _cdk_gettid(void) {

	uint64_t tid;
	pthread_threadid_np(NULL, &tid);

	return tid;
}
#endif

#if defined(__linux__)
pid_t _cdk_gettid(void) {

	return syscall(SYS_gettid);
}
#endif

typedef struct _thrd_ctx {
	int     (*entry)(void* arg);
	void*   arg;
}thrd_ctx;

static void* _thread_start(void* arg) {

	thrd_ctx* ctxp;
	thrd_ctx  ctx;

	ctxp = arg;
	ctx  = *ctxp;

	cdk_free(ctxp);
	ctx.entry(ctx.arg);

	return NULL;
}

_Bool _cdk_thrd_create(thrd_t* t, int (*h)(void*), void* restrict p) {

	int        r;
	thrd_ctx*  ctx;

	ctx = cdk_malloc(sizeof(thrd_ctx));

	ctx->entry = h;
	ctx->arg   = p;

	r = pthread_create(t, NULL, _thread_start, ctx);
	switch (r) {
	case 0:
		return true;
	default:
		cdk_free(ctx);
		return false;
	}
}

_Bool _cdk_thrd_join(thrd_t t) {

	int r;
	r = pthread_join(t, NULL);
	switch (r) {
	case 0:
		return true;
	default:
		return false;
	}
}

_Bool _cdk_thrd_detach(thrd_t t) {

	int r;
	r = pthread_detach(t);
	switch (r) {
	case 0:
		return true;
	default:
		return false;
	}
}

_Bool _cdk_thrd_once(once_flag* f, void (*h)(void)) {
	
	int r;
	r = pthread_once(f, h);
	switch (r) {
	case 0:
		return true;
	default:
		return false;
	}
}
