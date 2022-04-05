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

#include "win-thread.h"
#include "cdk/cdk-memory.h"
#include <process.h>
#include <stdlib.h>
#include <errno.h>

typedef struct _thrd_ctx {
	int     (*entry)(void* arg);
	void*   arg;
}thrd_ctx;

typedef struct _once_ctx {
	void (*entry)(void);
}once_ctx;


DWORD _cdk_gettid(void) {

	return GetCurrentThreadId();
}

static unsigned int __stdcall _thread_start(void* arg) {

	thrd_ctx* ctxp;
	thrd_ctx  ctx;

	ctxp = arg;
	ctx  = *ctxp;

	cdk_free(ctxp);

	return ctx.entry(ctx.arg);
}

static BOOL __stdcall _once_start(PINIT_ONCE once, PVOID param, PVOID* context) {

	once_ctx* ctxp;
	once_ctx  ctx;

	ctxp = param;
	ctx  = *ctxp;

	cdk_free(ctxp);

	ctx.entry();

	return TRUE;
}

BOOL _cdk_thrd_create(thrd_t* t, int (*h)(void*), void* restrict p) {

	thrd_ctx*    ctx;
	HANDLE       thrd;

	ctx = cdk_malloc(sizeof(thrd_ctx));

	ctx->entry = h;
	ctx->arg   = p;

	thrd = (HANDLE)_beginthreadex(NULL, 0, _thread_start, ctx, 0, NULL);
	switch ((uintptr_t)thrd)
	{
	case 0:
		cdk_free(ctx);
		return FALSE;
	default:
		*t = thrd;
		return TRUE;
	}
}

BOOL _cdk_thrd_join(thrd_t t) {

	DWORD r;
	r = WaitForSingleObject(t, INFINITE);

	switch (r) {
	case WAIT_OBJECT_0:
		CloseHandle(t);
		return TRUE;
	default:
		return FALSE;
	}
}

BOOL _cdk_thrd_detach(thrd_t t) {

	CloseHandle(t);
	return TRUE;
}

BOOL _cdk_thrd_once(once_flag* f, void (*h)(void)) {
	
	BOOL r;
	once_ctx* ctx;

	ctx = cdk_malloc(sizeof(once_ctx));
	ctx->entry = h;

	r = InitOnceExecuteOnce(f, _once_start, ctx, NULL);
	switch (r) {
	case 0:
		return FALSE;
	default:
		return TRUE;
	}
}
