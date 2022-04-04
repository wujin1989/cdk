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

#include "cdk/cdk-thread.h"

#if defined(__linux__) || defined(__APPLE__)
#include "unix/unix-thread.h"
#endif

#if defined(_WIN32)
#include "win/win-thread.h"
#endif

tid_t cdk_gettid(void) {

	return _cdk_gettid();
}

bool cdk_thrd_create(thrd_t* t, int (*h)(void*), void* restrict p) {

	return _cdk_thrd_create(t, h, p);
}

bool cdk_thrd_join(thrd_t t) {

	return _cdk_thrd_join(t);
}

bool cdk_thrd_detach(thrd_t t) {

	return _cdk_thrd_detach(t);
}

bool cdk_thrd_once(once_flag *f, void (*h)(void)) {

	return _cdk_thrd_once(f, h);
}