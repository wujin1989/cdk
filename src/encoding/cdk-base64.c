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

#include <string.h>
#include <stdint.h>

void cdk_base64_encode(uint8_t* src, size_t srclen, uint8_t* dst, size_t* dstlen) {

	static const char b64[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	size_t i, j;

	for (i = 0, j = 0; i < srclen; i += 3, j += 4) {

		uint32_t n = ((uint32_t)src[i]) << 16;

		if (i + 1 < srclen) {
			n |= ((uint32_t)src[i + 1]) << 8;
		}
		if (i + 2 < srclen) {
			n |= ((uint32_t)src[i + 2]);
		}
		dst[j]     = b64[(n >> 18) & 63];
		dst[j + 1] = b64[(n >> 12) & 63];
		dst[j + 2] = (i + 1 < srclen) ? b64[(n >> 6) & 63] : '=';
		dst[j + 3] = (i + 2 < srclen) ? b64[n & 63] : '=';
	}
	*dstlen = j;
}

void cdk_base64_decode(uint8_t* src, size_t srclen, uint8_t* dst, size_t* dstlen) {

    static const uint8_t b64[128] = {
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 0, 64, 64,
        64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 63,
        64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64
    };
    size_t i, j;

    for (i = 0, j = 0; i < srclen; i += 4, j += 3) {

        uint32_t n = (b64[src[i]] << 18)
                   | (b64[src[i + 1]] << 12)
                   | (b64[src[i + 2]] << 6)
                   | (b64[src[i + 3]]);

        dst[j] = (n >> 16) & 0xff;
        if (src[i + 2] != '=') {
            dst[j + 1] = (n >> 8) & 0xff;
        }
        if (src[i + 3] != '=') {
            dst[j + 2] = n & 0xff;
        }
    }
    *dstlen = j;
}

