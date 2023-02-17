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

#include "unix-sync.h"
#include <stdlib.h>

void _cdk_mtx_init(mtx_t* restrict m) {
	
	if (pthread_mutex_init(m, NULL)) { abort(); }
}

void _cdk_mtx_destroy(mtx_t* m) {

	if (pthread_mutex_destroy(m)) { 
		abort(); 
	}
}

void _cdk_mtx_lock(mtx_t* m) {

	if (pthread_mutex_lock(m)) { abort(); }
}

void _cdk_mtx_unlock(mtx_t* m) {

	if (pthread_mutex_unlock(m)) { abort(); }
}

void _cdk_cnd_init(cnd_t* restrict c) {

	if (pthread_cond_init(c, NULL)) { abort(); }
}

void _cdk_cnd_destroy(cnd_t* c) {

	if (pthread_cond_destroy(c)) { abort(); }
}

void _cdk_cnd_signal(cnd_t* c) {

	if (pthread_cond_signal(c)) { abort(); }
}

void _cdk_cnd_broadcast(cnd_t* c) {

	if (pthread_cond_broadcast(c)) { abort(); }
}

void _cdk_cnd_wait(cnd_t* restrict c, mtx_t* restrict m) {

	if (pthread_cond_wait(c, m)) { abort(); }
}
