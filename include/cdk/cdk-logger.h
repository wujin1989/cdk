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

#ifndef __CDK_LOGGER_H__
#define __CDK_LOGGER_H__

#include <stdbool.h>

enum {
	LEVEL_INFO    = 0,
	LEVEL_DEBUG   = 1,
	LEVEL_WARN    = 2,
	LEVEL_ERROR   = 3,
};

/**
 *  log output.
 *
 *  @param ... [in].
 *  @return N/A.
 */
#define cdk_logi(...)    cdk_log(LEVEL_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define cdk_logd(...)    cdk_log(LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define cdk_logw(...)    cdk_log(LEVEL_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define cdk_loge(...)    cdk_log(LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)

/**
 *  init logger engine.
 *
 *  @param o [in] if it is NULL, it means output to the console, otherwise output to the file specified by o.
 *  @param a [in] true means use asynchronous log, otherwise use synchronous log.
 *  @return N/A.
 */
extern void cdk_logger_init(const char* restrict o, bool a);

/**
 *  destroy logger engine.
 *
 *  @param N/A.
 *  @return N/A.
 */
extern void cdk_logger_destroy(void);
extern void cdk_log(int l, const char* restrict f, int n, const char* restrict fmt, ...);


#endif /* __CDK_LOGGER_H__ */
