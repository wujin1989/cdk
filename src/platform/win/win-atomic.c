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

_Bool platform_atomic_flag_test_and_set(volatile cdk_atomic_flag_t* flag) {

	return InterlockedExchange8((volatile char*)flag, 1) == 1;
}

void platform_atomic_flag_clear(volatile cdk_atomic_flag_t* flag) {

	InterlockedExchange8((volatile char*)flag, 0);
}

int64_t platform_atomic_load(const volatile cdk_atomic_t* obj) {

	return InterlockedCompareExchange64((volatile cdk_atomic_t*)obj, 0LL, 0LL);
}

void platform_atomic_store(volatile cdk_atomic_t* obj, int64_t desired) {
	
	InterlockedExchange64(obj, desired);
}

void platform_atomic_fetch_add(volatile cdk_atomic_t* obj, int64_t val) {

	InterlockedAdd64(obj, val);
}

void platform_atomic_fetch_sub(volatile cdk_atomic_t* obj, int64_t val) {

	InterlockedAdd64(obj, -val);
}

void platform_atomic_fetch_inc(volatile cdk_atomic_t* obj) {

	InterlockedAdd64(obj, 1LL);
}

void platform_atomic_fetch_dec(volatile cdk_atomic_t* obj) {

	InterlockedAdd64(obj, -1LL);
}

void platform_atomic_fetch_or(volatile cdk_atomic_t* obj, int64_t val) {

	InterlockedOr64(obj, val);
}

void platform_atomic_fetch_xor(volatile cdk_atomic_t* obj, int64_t val) {

	InterlockedXor64(obj, val);
}

void platform_atomic_fetch_and(volatile cdk_atomic_t* obj, int64_t val) {
	
	InterlockedAnd64(obj, val);
}

_Bool platform_atomic_cas(volatile cdk_atomic_t* obj, int64_t* expected, int64_t desired)
{
	LONG64 m = *expected;
	*expected = InterlockedCompareExchange64(obj, desired, m);
	
	return *expected == m;
}