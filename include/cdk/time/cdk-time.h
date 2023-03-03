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

#include <time.h>
#include <stdbool.h>
#include <stdint.h>

/**
 *  obtain a high-precision clock with a precision of milliseconds. instead of gettimeofday.
 * 
 *  @param N/A.
 *  @return if successful it returns current time in milliseconds since epoch. otherwise, it returns zero.
 */
extern uint64_t cdk_time_timespec_get(void);

/**
 *  convert the seconds that have passed since epoch to local time.
 *
 *  @param t [in] seconds elapsed since epoch.
 *  @param r [in/out] broken-down time.
 *  @return success return true; failure return false.
 */
extern bool cdk_time_localtime(const time_t* t, struct tm* r);

/**
 *  suspend current thread.
 *
 *  @param ms [in] milliseconds to suspend the current thread.
 *  @return N/A.
 */
extern void cdk_time_sleep(const uint32_t ms);

