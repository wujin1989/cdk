/** Copyright (c) 2023-2033, Wu Jin <wujin.developer@gmail.com>
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

_Bool platform_atomic_flag_test_and_set(volatile atomic_flag* f) {

	return InterlockedExchange8((volatile char*)f, 1) == 1;
}

void platform_atomic_flag_clear(volatile atomic_flag* f) {

	InterlockedExchange8((volatile char*)f, 0);
}

int64_t platform_atomic_load(const volatile atomic_t* t) {

	return InterlockedCompareExchange64((volatile atomic_t*)t, 0LL, 0LL);
}

void platform_atomic_store(volatile atomic_t* t, int64_t d) {
	
	InterlockedExchange64(t, d);
}

void platform_atomic_fetch_add(volatile atomic_t* t, int64_t o) {

	InterlockedAdd64(t, o);
}

void platform_atomic_fetch_sub(volatile atomic_t* t, int64_t o) {

	InterlockedAdd64(t, -o);
}

void platform_atomic_fetch_inc(volatile atomic_t* t) {

	InterlockedAdd64(t, 1LL);
}

void platform_atomic_fetch_dec(volatile atomic_t* t) {

	InterlockedAdd64(t, -1LL);
}

void platform_atomic_fetch_or(volatile atomic_t* t, int64_t o) {

	InterlockedOr64(t, o);
}

void platform_atomic_fetch_xor(volatile atomic_t* t, int64_t o) {

	InterlockedXor64(t, o);
}

void platform_atomic_fetch_and(volatile atomic_t* t, int64_t o) {
	
	InterlockedAnd64(t, o);
}

_Bool platform_atomic_cas(volatile atomic_t* t, int64_t* e, int64_t d)
{
	LONG64 m = *e;
	*e = InterlockedCompareExchange64(t, d, m);
	
	return *e == m;
}