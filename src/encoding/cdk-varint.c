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

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

int cdk_varint_encode(uint64_t value, char* buf) {
    int pos = 0;
    while (value > 0x7f) {
        buf[pos++] = (char)((value & 0x7f) | 0x80);
        value >>= 7;
    }
    buf[pos++] = (char)value;
    return pos;
}

uint64_t cdk_varint_decode(char* buf, int* pos) {
    uint64_t value = 0;
    int shift = 0;

    while (buf[*pos] & 0x80) {
        value |= (uint64_t)(buf[*pos] & 0x7f) << shift;
        shift += 7;
        (*pos)++;
    }
    value |= (uint64_t)(buf[(*pos)++]) << shift;
    return value;
}
