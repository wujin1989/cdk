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

_Pragma("once")

#include <cdk/cdk-types.h>

#if defined(_WIN32)
/**
 *  initializing an atomic flag.
 */
#define ATOMIC_FLAG_INIT            { 0 }
/**
 *  initializing an atomic object with value.
 */
#define ATOMIC_VAR_INIT(value)      (value)
#endif

/**
 *  atomically places the atomic flag pointed to by f in the set state.
 *
 *  @param f [in/out].
 *  @return preceding state.
 */
extern bool cdk_atomic_flag_test_and_set(volatile atomic_flag* f);

/**
 *  atomically places the atomic flag pointed to by f into the clear state.
 *
 *  @param f [in/out].
 *  @return N/A.
 */
extern void cdk_atomic_flag_clear(volatile atomic_flag* f);

/**
 *  atomically load the value pointed to by t.
 *
 *  @param t [in].
 *  @return the value pointed to by t.
 */
extern int64_t cdk_atomic_load(const volatile atomic_t* t);

/**
 *  atomically replace the value pointed to by t with the value of d.
 *
 *  @param t [in/out].
 *  @param d [in].
 *  @return N/A.
 */
extern void cdk_atomic_store(volatile atomic_t* t, int64_t d);

/**
 *  atomic addition.
 *
 *  @param t [in/out].
 *  @param o [in].
 *  @return N/A.
 */
extern void cdk_atomic_add(volatile atomic_t* t, int64_t o);

/**
 *  atomic subtraction.
 *
 *  @param t [in/out].
 *  @param o [in].
 *  @return N/A.
 */
extern void cdk_atomic_sub(volatile atomic_t* t, int64_t o);

/**
 *  atomic increment.
 *
 *  @param t [in/out].
 *  @return N/A.
 */
extern void cdk_atomic_inc(volatile atomic_t* t);

/**
 *  atomic decrement.
 *
 *  @param t  [in/out].
 *  @return N/A.
 */
extern void cdk_atomic_dec(volatile atomic_t* t);

/**
 *  atomic bitwise or.
 *
 *  @param t [in/out].
 *  @param o [in].
 *  @return N/A.
 */
extern void cdk_atomic_or(volatile atomic_t* t, int64_t o);

/**
 *  atomic bitwise xor.
 *
 *  @param t [in/out].
 *  @param o [in].
 *  @return N/A.
 */
extern void cdk_atomic_xor(volatile atomic_t* t, int64_t o);

/**
 *  atomic bitwise and.
 *
 *  @param t [in/out].
 *  @param o [in].
 *  @return N/A.
 */
extern void cdk_atomic_and(volatile atomic_t* t, int64_t o);

/**
 *  atomically, compares the contents of the memory pointed to by t for equality with that
 *  pointed to by e, and if true, replaces the contents of the memory pointed to by t
 *  with d, and if false, updates the contents of the memory pointed to by e with that 
 *  pointed to by t.
 *
 *  @param t [in/out].
 *  @param e [in/out].
 *  @param d [in].
 *  @return if t is equals to e, return true. otherwise return fase.
 */
extern bool cdk_atomic_cas(volatile atomic_t* t, int64_t* e, int64_t d);

