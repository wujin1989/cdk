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

#include "cdk/cdk-logger.h"
#include "cdk/cdk-threadpool.h"
#include "cdk/cdk-time.h"
#include "cdk/deprecated/c11-threads.h"
#include "platform/platform-io.h"
#include "platform/platform-utils.h"
#include <stdarg.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__linux__) || defined(__APPLE__)
#define S '/'
#endif

#if defined(_WIN32)
#define S '\\'
#endif

#define BUFSIZE 4096
#define DEFAULT_LOGGER_THREAD_NUM 1

typedef struct logger_s {
    FILE*              out;
    _Bool              async;
    mtx_t              mtx;
    cdk_logger_level_t level;
    cdk_logger_cb_t    callback;
    cdk_thrdpool_t     pool;
    atomic_flag        initialized;
    atomic_bool        closing;
} logger_t;

static const char* levels[] = {"DEBUG", "INFO", "WARN", "ERROR"};

static logger_t global_logger = {.initialized = ATOMIC_FLAG_INIT};

static inline void _printer(void* arg) {
    fprintf(global_logger.out, "%s", (char*)arg);
    fflush(global_logger.out);
    free(arg);
    arg = NULL;
}

static inline void _syncbase(
    cdk_logger_level_t level,
    const char* restrict file,
    int line,
    const char* restrict fmt,
    va_list v) {
    struct timespec tsc;
    struct tm       tm;
    if (!timespec_get(&tsc, TIME_UTC)) {
        return;
    }
    cdk_time_localtime(&tsc.tv_sec, &tm);

    mtx_lock(&global_logger.mtx);

    fprintf(
        global_logger.out,
        "%04d-%02d-%02d %02d:%02d:%02d.%03d %8d %5s %s:%d ",
        tm.tm_year + 1900,
        tm.tm_mon + 1,
        tm.tm_mday,
        tm.tm_hour,
        tm.tm_min,
        tm.tm_sec,
        (int)(tsc.tv_nsec / 1000000UL),
        (int)platform_utils_systemtid(),
        levels[level],
        file,
        line);
    vfprintf(global_logger.out, fmt, v);
    fflush(global_logger.out);

    mtx_unlock(&global_logger.mtx);
}

static inline void _asyncbase(
    cdk_logger_level_t level,
    const char* restrict file,
    int line,
    const char* restrict fmt,
    va_list v) {
    struct timespec tsc;
    struct tm       tm;
    char            buf[BUFSIZE];
    int             ret;
    if (!timespec_get(&tsc, TIME_UTC)) {
        return;
    }
    cdk_time_localtime(&tsc.tv_sec, &tm);
    memset(buf, 0, sizeof(buf));
    ret = sprintf(
        buf,
        "%04d-%02d-%02d %02d:%02d:%02d.%03d %8d %5s %s:%d ",
        tm.tm_year + 1900,
        tm.tm_mon + 1,
        tm.tm_mday,
        tm.tm_hour,
        tm.tm_min,
        tm.tm_sec,
        (int)(tsc.tv_nsec / 1000000UL),
        (int)platform_utils_systemtid(),
        levels[level],
        file,
        line);
    if (ret > sizeof(buf)) {
        abort();
    }
    ret += platform_io_vsprintf(buf + ret, sizeof(buf) - ret, fmt, v);
    ret++;
    char* arg = malloc(ret);
    if (arg) {
        memcpy(arg, buf, ret);
        cdk_thrdpool_post(&global_logger.pool, _printer, arg);
    }
}

static inline void _cbbase(
    cdk_logger_level_t level,
    const char* restrict file,
    int line,
    const char* restrict fmt,
    va_list v) {
    char buf[BUFSIZE];
    int  ret;

    memset(buf, 0, sizeof(buf));
    ret = sprintf(buf, "%s:%d ", file, line);
    if (ret > sizeof(buf)) {
        abort();
    }
    platform_io_vsprintf(buf + ret, sizeof(buf) - ret, fmt, v);
    global_logger.callback(level, buf);
}

void cdk_logger_create(cdk_logger_config_t* config) {
    if (!atomic_flag_test_and_set(&global_logger.initialized)) {
        if (config->level == LOGGER_LEVEL_BGN ||
            config->level == LOGGER_LEVEL_END) {
            global_logger.level = LOGGER_LEVEL_DEBUG;
        } else {
            global_logger.level = config->level;
        }
        if (config->callback) {
            global_logger.callback = config->callback;
            return;
        }
        if (!config->async) {
            mtx_init(&global_logger.mtx, mtx_plain);
        }
        if (config->async) {
            global_logger.async = true;
            cdk_thrdpool_create(&global_logger.pool, DEFAULT_LOGGER_THREAD_NUM);
        }
        if (!config->out) {
            global_logger.out = stdout;
            return;
        }
        global_logger.out = platform_io_fopen(config->out, "a+");
        atomic_init(&global_logger.closing, false);
    }
}

void cdk_logger_destroy(void) {
    if (atomic_flag_test_and_set(&global_logger.initialized)) {
        if (global_logger.async) {
            cdk_thrdpool_destroy(&global_logger.pool);
        }
        if (!global_logger.async) {
            mtx_destroy(&global_logger.mtx);
        }
        if (global_logger.out) {
            fclose(global_logger.out);
        }
        if (global_logger.callback) {
            global_logger.callback = NULL;
        }
        atomic_flag_clear(&global_logger.initialized);
        atomic_store(&global_logger.closing, true);
    }
}

void cdk_logger_log(
    cdk_logger_level_t level,
    const char* restrict file,
    int line,
    const char* restrict fmt,
    ...) {
    char* p = strrchr(file, S);

    if (atomic_load(&global_logger.closing)) {
        return;
    }
    if (global_logger.callback) {
        va_list v;
        va_start(v, fmt);
        if (!p) {
            _cbbase(level, file, line, fmt, v);
        }
        if (p) {
            _cbbase(level, ++p, line, fmt, v);
        }
        va_end(v);
        return;
    }
    if (!global_logger.out) {
        return;
    }
    if (global_logger.level > level) {
        return;
    }
    if (!global_logger.async) {
        va_list v;
        va_start(v, fmt);
        if (!p) {
            _syncbase(level, file, line, fmt, v);
        }
        if (p) {
            _syncbase(level, ++p, line, fmt, v);
        }
        va_end(v);
    }
    if (global_logger.async) {
        va_list v;
        va_start(v, fmt);
        if (!p) {
            _asyncbase(level, file, line, fmt, v);
        }
        if (p) {
            _asyncbase(level, ++p, line, fmt, v);
        }
        va_end(v);
    }
}
