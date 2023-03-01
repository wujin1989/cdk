/** Copyright (c) 2023-2033, Wu Jin <wujin.developer@gmail.com>
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

#include "cdk/cdk-time.h"
#include "cdk/cdk-sync.h"
#include "cdk/cdk-io.h"
#include "cdk/cdk-queue.h"
#include "cdk/cdk-thread.h"
#include "cdk/cdk-memory.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#if defined(__linux__) || defined(__APPLE__)
#define S       '/'
#endif

#if defined(_WIN32)
#define S       '\\'
#endif

#define BUFSIZE    4096

typedef struct _entry_s {
	fifo_node_t    n;
	char           d[];
}entry_t;

typedef struct _logger_s {
	FILE*    f;
	mtx_t    m;

	_Bool    a;
	fifo_t   q;
	thrd_t   w;
	_Bool    w_s;
	mtx_t    q_m;
	cnd_t    q_c;
}logger_t;

static const char* __l[] = {
	"INFO", "DEBUG", "WARN", "ERROR"
};

static logger_t __g;

static void __syncbase(int l, const char* restrict f, int n) {

	struct timespec tsc;
	struct tm       tm;

	if (!timespec_get(&tsc, TIME_UTC)) { return; }
	cdk_localtime(&tsc.tv_sec, &tm);

	fprintf(__g.f, "%04d-%02d-%02d %02d:%02d:%02d.%03d %5s %s:%d ",       \
		tm.tm_year + 1900,                                              \
		tm.tm_mon + 1,                                                  \
		tm.tm_mday,                                                     \
		tm.tm_hour,                                                     \
		tm.tm_min,                                                      \
		tm.tm_sec,                                                      \
		(int)(tsc.tv_nsec / 1000000UL),                                 \
		__l[l],                                                          \
		f, n
	);
}

static void __asyncbase(int l, const char* restrict f, int n, const char* restrict fmt, va_list v) {

	struct timespec tsc;
	struct tm       tm;
	char            b[BUFSIZE];
	int             r;

	if (!timespec_get(&tsc, TIME_UTC)) { return; }
	cdk_localtime(&tsc.tv_sec, &tm);

	memset(b, 0, sizeof(b));
	r = cdk_sprintf(b, sizeof(b),                                      \
		"%04d-%02d-%02d %02d:%02d:%02d.%03d %5s %s:%d ",               \
		tm.tm_year + 1900,                                             \
		tm.tm_mon + 1,                                                 \
		tm.tm_mday,                                                    \
		tm.tm_hour,                                                    \
		tm.tm_min,                                                     \
		tm.tm_sec,                                                     \
		(int)(tsc.tv_nsec / 1000000UL),                                \
		__l[l],                                                         \
		f, n
	);
	r += cdk_vsprintf(b + r, sizeof(b) - r, fmt, v);
	r++;
	entry_t* e = cdk_malloc(sizeof(entry_t) + r);

	memcpy(e->d, b, r);

	cdk_mtx_lock(&__g.q_m);
	cdk_queue_enqueue(&__g.q, &e->n);
	cdk_cnd_signal(&__g.q_c);
	cdk_mtx_unlock(&__g.q_m);
}

static int __printer(void* p) {
	
	while (__g.w_s) {

		cdk_mtx_lock(&__g.q_m);
		entry_t* e_p;
		while (__g.w_s && cdk_queue_empty(&__g.q)) {
			cdk_cnd_wait(&__g.q_c, &__g.q_m);
		}
		e_p = cdk_queue_data(cdk_queue_dequeue(&__g.q), entry_t, n);
		cdk_mtx_unlock(&__g.q_m);
		if (e_p) {
			fprintf(__g.f, "%s", e_p->d);
			fflush(__g.f);
			cdk_free(e_p);
		}
	}
	return 0;
}

void cdk_logger_init(const char* restrict o, bool a) {

	if (a) {
		__g.w_s = true;
		cdk_mtx_init(&__g.q_m);
		cdk_cnd_init(&__g.q_c);
		cdk_queue_create(&__g.q);
		cdk_thrd_create(&__g.w, __printer, NULL);
	}
	__g.a = a;

	if (!o) { 
		__g.f = stdout;
		cdk_mtx_init(&__g.m);
		return; 
	}
	cdk_io_fopen(&__g.f, o, "a+");

	cdk_mtx_init(&__g.m);
}

void cdk_logger_destroy(void) {

	if (__g.a) {
		__g.w_s = false;

		cdk_mtx_lock(&__g.q_m);
		cdk_cnd_broadcast(&__g.q_c);
		cdk_mtx_unlock(&__g.q_m);

		cdk_thrd_join(__g.w);

		cdk_mtx_destroy(&__g.q_m);
		cdk_cnd_destroy(&__g.q_c);
	}
	if (__g.f) { 
		cdk_io_fclose(__g.f); 
	}
	cdk_mtx_destroy(&__g.m);
}

void cdk_logger_log(int l, const char* restrict f, int n, const char* restrict fmt, ...) {
	
	char* p = strrchr(f, S);

	if (!__g.a) {
		
		cdk_mtx_lock(&__g.m);
		if (!p) { __syncbase(l, f, n); }
		if (p)  { __syncbase(l, ++p, n); }

		va_list v;
		va_start(v, fmt);
		vfprintf(__g.f, fmt, v);
		va_end(v);

		fflush(__g.f);
		cdk_mtx_unlock(&__g.m);
	}

	if (__g.a) {

		cdk_mtx_lock(&__g.m);
		va_list v;
		va_start(v, fmt);
		if (!p) { __asyncbase(l, f, n, fmt, v); }
		if (p)  { __asyncbase(l, ++p, n, fmt, v); }
		va_end(v);
		cdk_mtx_unlock(&__g.m);
	}
}
