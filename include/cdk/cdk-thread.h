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

#ifndef __CDK_THREAD_H__
#define __CDK_THREAD_H__

#include "cdk-types.h"

#if defined(__linux__) || defined(__APPLE__)
#define thread_local            __thread
#define ONCE_FLAG_INIT          PTHREAD_ONCE_INIT
#endif

#if defined(_WIN32)
/**
 *  thread-specific storage modifier.
 */
#define thread_local            __declspec(thread)
/**
 *  used to initialize an object of type once_flag.
 */
#define ONCE_FLAG_INIT          INIT_ONCE_STATIC_INIT
#endif

/**
 *  get current thread-id in process.
 *
 *  @param N/A.
 *  @return thread-id.
 */
extern tid_t cdk_thrd_gettid(void);

/**
 *  create a new thread.
 *
 *  @param t [in/out] thread identifier.
 *  @param h [in].
 *  @param p [in] thread's parameter.
 *  @return success return true; failure return false.
 */
extern void cdk_thrd_create(thrd_t* t, int (*h)(void*), void* restrict p);

/**
 *  waits for the thread specified by thread to terminate.
 *
 *  @param t [in] thread identifier.
 *  @return success return true; failure return false.
 */
extern void cdk_thrd_join(thrd_t t);

/**
 *  marks the thread identified by thread as detached. When a detached thread terminates,
 *  its resources are automatically released back to the system.
 *
 *  @param t [in] thread identifier.
 *  @return success return true; failure return false.
 */
extern void cdk_thrd_detach(thrd_t t);

/**
 *  in the case of multiple threads, the specified function is executed only once.
 *
 *  @param f [in] once control flag, initialize with ONCE_FLAG_INIT.
 *  @param h [in].
 *  @return success return true; failure return false.
 */
extern void cdk_thrd_once(once_flag* f, void (*h)(void));

#endif /* __CDK_THREAD_H__ */
