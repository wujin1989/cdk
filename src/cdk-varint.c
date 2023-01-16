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

#include "cdk/cdk-varint.h"
#include <string.h>
#include <stdlib.h>

static const int8_t MSB    = 0x80;
static const int8_t MSBALL = ~0x7F;

static const uint64_t N1 = 128;
static const uint64_t N2 = 16384;
static const uint64_t N3 = 2097152;
static const uint64_t N4 = 268435456;
static const uint64_t N5 = 34359738368;
static const uint64_t N6 = 4398046511104;
static const uint64_t N7 = 562949953421312;
static const uint64_t N8 = 72057594037927936;
static const uint64_t N9 = 9223372036854775808U;

size_t cdk_varint_encode(uint64_t num, char* buf, size_t len) {

    char* ptr = buf;

    while (num & MSBALL) {
        *(ptr++) = (num & 0xFF) | MSB;
        num = num >> 7;
        if ((ptr - buf) >= len) {
            abort();
        };
    }
    *ptr = num;

    return ptr - buf + 1;
}

uint64_t cdk_varint_decode(char* buf, size_t* len) {

    uint64_t result, ll;
    int      bits;
    char*    ptr;

    result = bits = ll = 0;
    ptr = buf;

    while (*ptr & MSB) {
        ll = *ptr;
        result += ((ll & 0x7F) << bits);
        ptr++;
        bits += 7;
        if ((ptr - buf) >= len) {
            abort();
        };
    }
    ll = *ptr;
    result += ((ll & 0x7F) << bits);

    if (len != NULL) {
        *len = ptr - buf + 1;
    }
    return result;
}

size_t cdk_varint_encoding_length(uint64_t num) {

    return (
        num < N1 ? 1
        : num < N2 ? 2
        : num < N3 ? 3
        : num < N4 ? 4
        : num < N5 ? 5
        : num < N6 ? 6
        : num < N7 ? 7
        : num < N8 ? 8
        : num < N9 ? 9
        : 10
        );
}