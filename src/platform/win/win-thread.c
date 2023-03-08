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

#include "cdk/cdk-memory.h"
#include "cdk/cdk-types.h"
#include <process.h>
#include <stdlib.h>
#include <errno.h>

typedef struct platform_thrd_ctx_s {
	int     (*entry)(void* arg);
	void*   arg;
}platform_thrd_ctx_t;

typedef struct platform_once_ctx_s {
	void (*entry)(void);
}platform_once_ctx_t;

static unsigned int __stdcall platform_thrd_start(void* arg) {

	platform_thrd_ctx_t* ctxp;
	platform_thrd_ctx_t  ctx;

	ctxp = arg;
	ctx  = *ctxp;

	cdk_memory_free(ctxp);

	return ctx.entry(ctx.arg);
}

static BOOL __stdcall platform_once_start(PINIT_ONCE once, PVOID param, PVOID* context) {

	platform_once_ctx_t* ctxp;
	platform_once_ctx_t  ctx;

	ctxp = param;
	ctx  = *ctxp;

	cdk_memory_free(ctxp);

	ctx.entry();

	return TRUE;
}

cdk_tid_t platform_thrd_gettid(void) {

	return GetCurrentThreadId();
}

void platform_thrd_create(cdk_thrd_t* t, int (*h)(void*), void* restrict p) {

	platform_thrd_ctx_t* ctx;
	HANDLE hnd;

	ctx = cdk_memory_malloc(sizeof(platform_thrd_ctx_t));

	ctx->entry = h;
	ctx->arg   = p;

	hnd = (HANDLE)_beginthreadex(NULL, 0, platform_thrd_start, ctx, 0, &t->tid);

	switch ((uintptr_t)hnd)
	{
	case 0:
		cdk_memory_free(ctx);
		return;
	default:
		t->handle = hnd;
		return;
	}
}

void platform_thrd_join(cdk_thrd_t t) {

	if (WAIT_OBJECT_0 == WaitForSingleObject(t.handle, INFINITE)) {
		CloseHandle(t.handle);
	}
}

void platform_thrd_detach(cdk_thrd_t t) {

	CloseHandle(t.handle);
}

void platform_thrd_once(cdk_once_t* f, void (*h)(void)) {
	
	platform_once_ctx_t* ctx;

	ctx = cdk_memory_malloc(sizeof(platform_once_ctx_t));
	ctx->entry = h;

	InitOnceExecuteOnce(f, platform_once_start, ctx, NULL);
}

bool platform_thrd_equal(cdk_thrd_t t1, cdk_thrd_t t2) {

	return t1.tid == t2.tid;
}

cdk_thrd_t platform_thrd_current(void) {

	cdk_thrd_t t;

	t.handle = GetCurrentThread();
	t.tid    = GetCurrentThreadId();

	return t;
}