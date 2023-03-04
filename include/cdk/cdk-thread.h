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

_Pragma("once")

#include "cdk-types.h"

#if defined(__linux__) || defined(__APPLE__)
#define cdk_thread_local            thread_local
#define CDK_ONCE_FLAG_INIT          PTHREAD_ONCE_INIT
#endif

#if defined(_WIN32)
#define cdk_thread_local            __declspec(thread)
#define CDK_ONCE_FLAG_INIT          INIT_ONCE_STATIC_INIT
#endif


extern cdk_tid_t  cdk_thrd_gettid(void);
extern void		  cdk_thrd_create(cdk_thrd_t* t, int (*h)(void*), void* restrict p);
extern void		  cdk_thrd_join(cdk_thrd_t t);
extern void		  cdk_thrd_detach(cdk_thrd_t t);
extern void		  cdk_thrd_once(cdk_once_t* f, void (*h)(void));
extern bool       cdk_thrd_equal(cdk_thrd_t t1, cdk_thrd_t t2);
extern cdk_thrd_t cdk_thrd_current(void);

