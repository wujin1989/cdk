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

#include "win-sync.h"

void _mtx_init(mtx_t* restrict m) {
	
	InitializeCriticalSection(m);
}

void _mtx_destroy(mtx_t* m) {
	
	DeleteCriticalSection(m);
}

void _mtx_lock(mtx_t* m) {
	
	EnterCriticalSection(m);
}
void _mtx_unlock(mtx_t* m) {

	LeaveCriticalSection(m);
}

void _cnd_init(cnd_t* restrict c) {

	InitializeConditionVariable(c);
}

void _cnd_destroy(cnd_t* c) {

}

void _cnd_signal(cnd_t* c) {

	WakeConditionVariable(c);
}

void _cnd_broadcast(cnd_t* c) {

	WakeAllConditionVariable(c);
}

void _cnd_wait(cnd_t* restrict c, mtx_t* restrict m) {

	if (SleepConditionVariableCS(c, m, INFINITE) == 0) {
		abort();
	}
}