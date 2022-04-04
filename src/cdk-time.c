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

#include "cdk/cdk-time.h"

#if defined(__linux__) || defined(__APPLE__)
#include "unix/unix-time.h"
#endif

#if defined(_WIN32)
#include "win/win-time.h"
#endif

#define USEC                        (1000000UL)
#define MSEC                        (1000UL)

uint64_t cdk_timespec_get(void) {

	struct timespec tsc;
	if (!timespec_get(&tsc, TIME_UTC)) { return 0; }

	return (tsc.tv_sec * MSEC + tsc.tv_nsec / USEC);
}

bool cdk_localtime(const time_t* t, struct tm* r) {

	return _cdk_localtime(t, r);
}

void cdk_sleep(const uint32_t ms) {

	_cdk_sleep(ms);
}