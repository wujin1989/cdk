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

#include "cdk/cdk-types.h"

#define CDK_ATOMIC_FLAG_INIT            { 0 }
#define CDK_ATOMIC_VAR_INIT(value)      (value)

extern bool    cdk_atomic_flag_test_and_set(volatile cdk_atomic_flag* f);
extern void	   cdk_atomic_flag_clear(volatile cdk_atomic_flag* f);
extern int64_t cdk_atomic_load(const volatile cdk_atomic_t* t);
extern void    cdk_atomic_store(volatile cdk_atomic_t* t, int64_t d);
extern void    cdk_atomic_add(volatile cdk_atomic_t* t, int64_t o);
extern void    cdk_atomic_sub(volatile cdk_atomic_t* t, int64_t o);
extern void    cdk_atomic_inc(volatile cdk_atomic_t* t);
extern void    cdk_atomic_dec(volatile cdk_atomic_t* t);
extern void    cdk_atomic_or(volatile cdk_atomic_t* t, int64_t o);
extern void    cdk_atomic_xor(volatile cdk_atomic_t* t, int64_t o);
extern void    cdk_atomic_and(volatile cdk_atomic_t* t, int64_t o);
extern bool    cdk_atomic_cas(volatile cdk_atomic_t* t, int64_t* e, int64_t d);

