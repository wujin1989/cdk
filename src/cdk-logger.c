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

#include "cdk/cdk-time.h"
#include "cdk/cdk-string.h"
#include "cdk/cdk-file.h"
#include "cdk/cdk-threadpool.h"
#include "cdk/cdk-memory.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#if defined(__linux__) || defined(__APPLE__)
#define S       '/'
#endif

#if defined(_WIN32)
#define S       '\\'
#endif

#define BUFSIZE    4096

typedef struct logger_s {
	FILE* file;
	_Bool async;
	cdk_thrdpool_t pool;
}logger_t;

static const char* __l[] = {
	"INFO", "DEBUG", "WARN", "ERROR"
};

static logger_t __g;

static void __printer(void* arg) {

	fprintf(__g.file, "%s", (char*)arg);
	fflush(__g.file);
	cdk_memory_free(arg);
}

static void __syncbase(int level, const char* restrict file, int line) {

	struct timespec tsc;
	struct tm       tm;

	if (!timespec_get(&tsc, TIME_UTC)) { return; }
	cdk_time_localtime(&tsc.tv_sec, &tm);

	fprintf(__g.file, "%04d-%02d-%02d %02d:%02d:%02d.%03d %5s %s:%d ",  \
		tm.tm_year + 1900,                                              \
		tm.tm_mon + 1,                                                  \
		tm.tm_mday,                                                     \
		tm.tm_hour,                                                     \
		tm.tm_min,                                                      \
		tm.tm_sec,                                                      \
		(int)(tsc.tv_nsec / 1000000UL),                                 \
		__l[level],                                                     \
		file, line
	);
}

static void __asyncbase(int level, const char* restrict file, int line, const char* restrict fmt, va_list v) {

	struct timespec tsc;
	struct tm       tm;
	char            buf[BUFSIZE];
	int             ret;

	if (!timespec_get(&tsc, TIME_UTC)) {
		return;
	}
	cdk_time_localtime(&tsc.tv_sec, &tm);

	memset(buf, 0, sizeof(buf));
	ret = cdk_string_sprintf(buf, sizeof(buf),                         \
		"%04d-%02d-%02d %02d:%02d:%02d.%03d %5s %s:%d ",               \
		tm.tm_year + 1900,                                             \
		tm.tm_mon + 1,                                                 \
		tm.tm_mday,                                                    \
		tm.tm_hour,                                                    \
		tm.tm_min,                                                     \
		tm.tm_sec,                                                     \
		(int)(tsc.tv_nsec / 1000000UL),                                \
		__l[level],                                                    \
		file, line
	);
	ret += cdk_string_vsprintf(buf + ret, sizeof(buf) - ret, fmt, v);
	ret++;

	cdk_thrdpool_job_t* job = cdk_memory_malloc(sizeof(cdk_thrdpool_job_t));
	job->routine = __printer;
	job->arg = cdk_memory_malloc(ret);
	memcpy(job->arg, buf, ret);

	cdk_thrdpool_post(&__g.pool, job);
}

void cdk_logger_init(const char* restrict out, bool async, int workers) {

	if (async) {
		cdk_thrdpool_create(&__g.pool, workers);
	}
	__g.async = async;

	if (!out) { 
		__g.file = stdout;
		return; 
	}
	cdk_file_fopen(&__g.file, out, "a+");
}

void cdk_logger_destroy(void) {

	if (__g.async) {
		cdk_thrdpool_destroy(&__g.pool);
	}
	if (__g.file) { 
		cdk_file_fclose(__g.file); 
	}
}

void cdk_logger_log(int level, const char* restrict file, int line, const char* restrict fmt, ...) {
	
	char* p = strrchr(file, S);

	if (!__g.async) {
		
		if (!p) {
			__syncbase(level, file, line);
		}
		if (p) {
			__syncbase(level, ++p, line);
		}
		va_list v;
		va_start(v, fmt);
		vfprintf(__g.file, fmt, v);
		va_end(v);

		fflush(__g.file);
	}
	if (__g.async) {

		va_list v;
		va_start(v, fmt);

		if (!p) {
			__asyncbase(level, file, line, fmt, v);
		}
		if (p) {
			__asyncbase(level, ++p, line, fmt, v);
		}
		va_end(v);
	}
}
