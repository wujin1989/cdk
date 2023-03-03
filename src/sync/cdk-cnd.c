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

#include "platform-cnd.h"
#include "cdk/cdk-types.h"

void cdk_cnd_init(cdk_cnd_t* restrict c) {

	platform_cnd_init(c);
}

void cdk_cnd_destroy(cdk_cnd_t* c) {

	platform_cnd_destroy(c);
}

void cdk_cnd_signal(cdk_cnd_t* c) {

	platform_cnd_signal(c);
}

void cdk_cnd_broadcast(cdk_cnd_t* c) {

	platform_cnd_broadcast(c);
}

void cdk_cnd_wait(cdk_cnd_t* restrict c, cdk_mtx_t* restrict m) {

	platform_cnd_wait(c, m);
}