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
#define DEFAULT_LOGGER_THREAD_NUM 1

typedef struct logger_s {
	FILE* file;
	_Bool async;
	cdk_thrdpool_t pool;
}logger_t;

static const char* levels[] = {
	"INFO", "DEBUG", "WARN", "ERROR"
};

static logger_t logger;
static atomic_flag initialized = ATOMIC_FLAG_INIT;

static inline void _logger_printer(void* arg) {
	fprintf(logger.file, "%s", (char*)arg);
	fflush(logger.file);
	free(arg);
	arg = NULL;
}

static inline void _logger_syncbase(int level, const char* restrict file, int line) {
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

static inline void _logger_asyncbase(int level, const char* restrict file, int line, const char* restrict fmt, va_list v) {
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
	char* arg = malloc(ret);
	if (arg) {
		memcpy(arg, buf, ret);
		cdk_thrdpool_post(&logger.pool, _logger_printer, arg);
	}
}

void cdk_logger_create(const char* restrict out, bool async) {
	if (!atomic_flag_test_and_set(&initialized)) {
		if (async) {
			logger.async = true;
			cdk_thrdpool_create(&logger.pool, DEFAULT_LOGGER_THREAD_NUM);
		}
		if (!out) {
			logger.file = stdout;
			return;
		}
		logger.file = fopen(out, "a+");
	}
}

void cdk_logger_destroy(void) {
	if (atomic_flag_test_and_set(&initialized)) {
		if (logger.async) {
			cdk_thrdpool_destroy(&logger.pool);
		}
		if (logger.file) {
			fclose(logger.file);
		}
		atomic_flag_clear(&initialized);
	}
}

void cdk_logger_log(int level, const char* restrict file, int line, const char* restrict fmt, ...) {
	if (logger.file == NULL) {
		return;
	}
	char* p = strrchr(file, S);
	if (!logger.async) {
		if (!p) {
			_logger_syncbase(level, file, line);
		}
		if (p) {
			_logger_syncbase(level, ++p, line);
		}
		va_list v;
		va_start(v, fmt);
		vfprintf(logger.file, fmt, v);
		va_end(v);

		fflush(logger.file);
	}
	if (logger.async) {
		va_list v;
		va_start(v, fmt);
		if (!p) {
			_logger_asyncbase(level, file, line, fmt, v);
		}
		if (p) {
			_logger_asyncbase(level, ++p, line, fmt, v);
		}
		va_end(v);
	}
}
