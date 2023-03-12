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

#include "cdk/cdk-types.h"
#include "platform-thread.h"

cdk_tid_t cdk_thrd_gettid(void) {

	return platform_thrd_gettid();
}

void cdk_thrd_create(cdk_thrd_t* t, int (*h)(void*), void* restrict p) {

	platform_thrd_create(t, h, p);
}

void cdk_thrd_join(cdk_thrd_t t) {

	platform_thrd_join(t);
}

void cdk_thrd_detach(cdk_thrd_t t) {

	platform_thrd_detach(t);
}

void cdk_thrd_once(cdk_once_t *f, void (*h)(void)) {

	platform_thrd_once(f, h);
}

bool cdk_thrd_equal(cdk_thrd_t t1, cdk_thrd_t t2) {

	return platform_thrd_equal(t1, t2);
}

cdk_thrd_t cdk_thrd_current(void) {

	return platform_thrd_current();
}