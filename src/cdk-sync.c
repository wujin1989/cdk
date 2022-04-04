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

#include "cdk/cdk-sync.h"
#include "cdk/cdk-atomic.h"

#if defined(__linux__) || defined(__APPLE__)
#include "unix/unix-sync.h"
#endif

#if defined(_WIN32)
#include "win/win-sync.h"
#endif

void cdk_mtx_init(mtx_t* restrict m) {

	_cdk_mtx_init(m);
}

void cdk_mtx_destroy(mtx_t* m) {
	
	_cdk_mtx_destroy(m);
}

void cdk_mtx_lock(mtx_t* m) {
	
	_cdk_mtx_lock(m);
}

void cdk_mtx_unlock(mtx_t* m) {

	 _cdk_mtx_unlock(m);
}

void cdk_rwlock_init(rwlock_t* restrict rw) {
	
	rw->w = ATOMIC_VAR_INIT(0);
	rw->r = ATOMIC_VAR_INIT(0);
}

void cdk_rwlock_destroy(rwlock_t* rw) {

}

void cdk_rwlock_rlock(rwlock_t* rw) {
	
	for (;;) {

		while (cdk_atomic_load(&rw->w)) {}
		cdk_atomic_inc(&rw->r);

		if (cdk_atomic_load(&rw->w)) {
			cdk_atomic_dec(&rw->r);
		} else {
			break;
		}
	}
}

void cdk_rwlock_runlock(rwlock_t* rw) {

	cdk_atomic_dec(&rw->r);
}

void cdk_rwlock_wlock(rwlock_t* rw) {

	int64_t c = 0;
	while (!cdk_atomic_cas(&rw->w, &c, 1)) { c = 0; }
	while (cdk_atomic_load(&rw->r)) {}
}

void cdk_rwlock_wunlock(rwlock_t* rw) {

	cdk_atomic_store(&rw->w, 0);
}

void cdk_cnd_init(cnd_t* restrict c) {

	_cdk_cnd_init(c);
}

void cdk_cnd_destroy(cnd_t* c) {

	_cdk_cnd_destroy(c);
}

void cdk_cnd_signal(cnd_t* c) {

	_cdk_cnd_signal(c);
}

void cdk_cnd_broadcast(cnd_t* c) {

	_cdk_cnd_broadcast(c);
}

void cdk_cnd_wait(cnd_t* restrict c, mtx_t* restrict m) {

	_cdk_cnd_wait(c, m);
}