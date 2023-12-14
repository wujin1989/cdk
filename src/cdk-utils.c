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

#include "platform/platform-utils.h"
#include <time.h>
#include <stdlib.h>

int cdk_utils_cpus(void) {
	return (int)platform_utils_cpus();
}

cdk_tid_t cdk_utils_systemtid(void) {
    return platform_utils_systemtid();
}

int cdk_utils_byteorder(void) {
	return (*((unsigned char*)(&(unsigned short){ 0x01 })));
}

int cdk_utils_rand(int min, int max) {
    static unsigned int s = 0;
    if (s == 0) {
        s = (unsigned int)time(NULL);
        srand(s);
    }
    return min + (int)((double)((double)(max)-(min)+1.0) * (rand() / ((RAND_MAX)+1.0)));
}