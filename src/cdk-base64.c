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
#include "cdk/cdk-memory.h"

static const uint8_t base64table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

void cdk_base64_encode(uint8_t* src, size_t srclen, char* dst, size_t* dstlen) {

	size_t i;
	size_t leven = 3 * (srclen / 3);

	for (*dstlen = 0, i = 0; i < leven; i += 3) {

		if (dst) {

			dst[*dstlen] = base64table[src[0] >> 2];
			dst[*dstlen + 1] = base64table[((src[0] & 3) << 4) + (src[1] >> 4)];
			dst[*dstlen + 2] = base64table[((src[1] & 0xf) << 2) + (src[2] >> 6)];
			dst[*dstlen + 3] = base64table[src[2] & 0x3f];
		}
		*dstlen += 4;
		src += 3;
	}
	if (i < srclen) {

		uint8_t  a = src[0];
		uint8_t  b = (uint8_t)((i + 1 < srclen) ? src[1] : 0);
		uint32_t c = 0;

		if (dst) {

			dst[*dstlen] = base64table[a >> 2];
			dst[*dstlen + 1] = base64table[((a & 3) << 4) + (b >> 4)];
			dst[*dstlen + 2] = (char)((i + 1 < srclen) ? base64table[((b & 0xf) << 2) + (c >> 6)] : '=');
			dst[*dstlen + 3] = '=';
		}
		*dstlen += 4;
	}
	if (dst) {
		dst[*dstlen] = 0;
	}
}

static uint8_t cdk_base64_byte(char c) {

	if (c >= '0' && c <= '9') {
		return (uint8_t)(c - '0' + 52);
	}
	if (c >= 'A' && c <= 'Z') {
		return (uint8_t)(c - 'A');
	}
	if (c >= 'a' && c <= 'z') {
		return (uint8_t)(c - 'a' + 26);
	}
	if ('+' == c) {
		return 62;
	}
	if ('/' == c) {
		return 63;
	}
	return 64;
}

void cdk_base64_decode(char* src, size_t srclen, uint8_t* dst, size_t* dstlen) {

	uint8_t input[4];
	size_t i;

	for (*dstlen = 0, i = 0; i < srclen; i += 4) {

		if (dst) {

			input[0] = cdk_base64_byte(src[i]);
			input[1] = cdk_base64_byte(src[i + 1]);
			dst[*dstlen] = (input[0] << 2) + (input[1] >> 4);
		}
		++(*dstlen);
		if (src[i + 2] != '=') {

			if (dst) {
				input[2] = cdk_base64_byte(src[i + 2]);
				dst[*dstlen] = (input[1] << 4) + (input[2] >> 2);
			}
			++(*dstlen);
		}
		if (src[i + 3] != '=') {

			if (dst) {
				input[3] = cdk_base64_byte(src[i + 3]);
				dst[*dstlen] = (input[2] << 6) + input[3];
			}
			++(*dstlen);
		}
	}
}