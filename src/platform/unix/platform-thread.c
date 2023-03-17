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

#include "cdk.h"

#if defined(__APPLE__)
cdk_tid_t platform_thread_gettid(void) {

	uint64_t tid;
	pthread_threadid_np(NULL, &tid);

	return tid;
}
#endif

#if defined(__linux__)
cdk_tid_t platform_thread_gettid(void) {

	return syscall(SYS_gettid);
}
#endif

void platform_thread_rwlock_init(cdk_rwlock_t* restrict rw) {

	if (pthread_rwlock_init(rw, NULL)) {
		abort();
	}
}

void platform_thread_rwlock_destroy(cdk_rwlock_t* rw) {
	
	if (pthread_rwlock_destroy(rw)) {
		abort();
	}
}

void platform_thread_rwlock_rlock(cdk_rwlock_t* rw) {

	if (pthread_rwlock_rdlock(rw)) {
		abort();
	}
}

void platform_thread_rwlock_runlock(cdk_rwlock_t* rw) {

	if (pthread_rwlock_unlock(rw)) {
		abort();
	}
}

void platform_thread_rwlock_wlock(cdk_rwlock_t* rw) {

	if (pthread_rwlock_wrlock(rw)) {
		abort();
	}
}

void platform_thread_rwlock_wunlock(cdk_rwlock_t* rw) {

	if (pthread_rwlock_unlock(rw)) {
		abort();
	}
}