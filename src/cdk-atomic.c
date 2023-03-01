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

#include "platform/platform-atomic.h"

bool cdk_atomic_flag_test_and_set(volatile atomic_flag* f) {

	return _atomic_flag_test_and_set(f);
}

void cdk_atomic_flag_clear(volatile atomic_flag* f) {
	
	_atomic_flag_clear(f);
}

int64_t cdk_atomic_load(const volatile atomic_t* t) {
	
	return _atomic_load(t);
}

void cdk_atomic_store(volatile atomic_t* t, int64_t d) {

	_atomic_store(t, d);
}

void cdk_atomic_add(volatile atomic_t* t, int64_t o) {
	
	_atomic_fetch_add(t, o);
}

void cdk_atomic_sub(volatile atomic_t* t, int64_t o) {
	
	_atomic_fetch_sub(t, o);
}

void cdk_atomic_inc(volatile atomic_t* t) {
	
	_atomic_fetch_inc(t);
}

void cdk_atomic_dec(volatile atomic_t* t) {

	_atomic_fetch_dec(t);
}

void cdk_atomic_or(volatile atomic_t* t, int64_t o) {
	
	_atomic_fetch_or(t, o);
}

void cdk_atomic_xor(volatile atomic_t* t, int64_t o) {
	
	_atomic_fetch_xor(t, o);
}

void cdk_atomic_and(volatile atomic_t* t, int64_t o) {

	_atomic_fetch_and(t, o);
}

bool cdk_atomic_cas(volatile atomic_t* t, int64_t* e, int64_t d)
{
	return _atomic_cas(t, e, d);
}