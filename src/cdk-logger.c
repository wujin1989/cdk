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
#include "cdk/cdk-threadpool.h"
#include <stdatomic.h>
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

typedef struct cdk_logger_s {
	FILE* file;
	mtx_t mtx;
	_Bool async;
	cdk_thrdpool_t pool;
}cdk_logger_t;

static const char* levels[] = {
	"INFO", "DEBUG", "WARN", "ERROR"
};

static cdk_logger_t logger;
static atomic_flag once_create  = ATOMIC_FLAG_INIT;
static atomic_flag once_destroy = ATOMIC_FLAG_INIT;

static inline void cdk_logger_printer(void* arg) {

	fprintf(logger.file, "%s", (char*)arg);
	fflush(logger.file);
	free(arg);
	arg = NULL;
}

static inline void cdk_logger_syncbase(int level, const char* restrict file, int line) {

	struct timespec tsc;
	struct tm       tm;

	if (!timespec_get(&tsc, TIME_UTC)) { return; }
	cdk_time_localtime(&tsc.tv_sec, &tm);

	fprintf(logger.file, "%04d-%02d-%02d %02d:%02d:%02d.%03d %5s %s:%d ",  \
		tm.tm_year + 1900,                                                 \
		tm.tm_mon + 1,                                                     \
		tm.tm_mday,                                                        \
		tm.tm_hour,                                                        \
		tm.tm_min,                                                         \
		tm.tm_sec,                                                         \
		(int)(tsc.tv_nsec / 1000000UL),                                    \
		levels[level],                                                     \
		file, line
	);
}

static inline void cdk_logger_asyncbase(int level, const char* restrict file, int line, const char* restrict fmt, va_list v) {

	struct timespec tsc;
	struct tm       tm;
	char            buf[BUFSIZE];
	int             ret;

	if (!timespec_get(&tsc, TIME_UTC)) {
		return;
	}
	cdk_time_localtime(&tsc.tv_sec, &tm);

	memset(buf, 0, sizeof(buf));
	ret = sprintf(buf,                                                 \
		"%04d-%02d-%02d %02d:%02d:%02d.%03d %5s %s:%d ",               \
		tm.tm_year + 1900,                                             \
		tm.tm_mon + 1,                                                 \
		tm.tm_mday,                                                    \
		tm.tm_hour,                                                    \
		tm.tm_min,                                                     \
		tm.tm_sec,                                                     \
		(int)(tsc.tv_nsec / 1000000UL),                                \
		levels[level],                                                 \
		file, line
	);
	ret += vsprintf(buf + ret, fmt, v);
	ret++;

	cdk_thrdpool_job_t* job = malloc(sizeof(cdk_thrdpool_job_t));
	if (job) {
		job->routine = cdk_logger_printer;
		job->arg = malloc(ret);
		memcpy(job->arg, buf, ret);

		cdk_thrdpool_post(&logger.pool, job);
	}
}

void cdk_logger_create(const char* restrict out, int nthrds) {

	if (atomic_flag_test_and_set(&once_create)) {
		return;
	}
	mtx_init(&logger.mtx, mtx_plain);

	if (nthrds > 0) {
		logger.async = true;
		cdk_thrdpool_create(&logger.pool, nthrds);
	}
	if (!out) { 
		logger.file = stdout;
		return; 
	}
	logger.file = fopen(out, "a+");
}

void cdk_logger_destroy(void) {

	if (atomic_flag_test_and_set(&once_destroy)) {
		return;
	}
	if (logger.async) {
		cdk_thrdpool_destroy(&logger.pool);
	}
	if (logger.file) {
		fclose(logger.file);
	}
	mtx_destroy(&logger.mtx);
}

void cdk_logger_log(int level, const char* restrict file, int line, const char* restrict fmt, ...) {
	
	char* p = strrchr(file, S);

	mtx_lock(&logger.mtx);

	if (!logger.async) {
		
		if (!p) {
			cdk_logger_syncbase(level, file, line);
		}
		if (p) {
			cdk_logger_syncbase(level, ++p, line);
		}
		va_list v;
		va_start(v, fmt);
		vfprintf(logger.file, fmt, v);
		va_end(v);

		fflush(logger.file);
	}
	mtx_unlock(&logger.mtx);

	if (logger.async) {

		va_list v;
		va_start(v, fmt);

		if (!p) {
			cdk_logger_asyncbase(level, file, line, fmt, v);
		}
		if (p) {
			cdk_logger_asyncbase(level, ++p, line, fmt, v);
		}
		va_end(v);
	}
}
