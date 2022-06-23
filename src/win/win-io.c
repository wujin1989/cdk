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

#include "win-io.h"
#include <stdarg.h>
#include <stdlib.h>
#include <share.h>
#include <string.h>

void _cdk_fopen(FILE** fp, const char* restrict f, const char* restrict m) {

	*fp = _fsopen(f, m, _SH_DENYNO);
	if (!*fp) {
		abort();
	}
}

int _cdk_sprintf(char* s, size_t sz, const char* f, va_list v) {

	return vsprintf_s(s, sz, f, v);
}

char* _cdk_strtok(char* s, const char* d, char** c) {

	return strtok_s(s, d, c);
}

void  _cdk_strcat(char* d, size_t n, const char* s) {

	strcat_s(d, n, s);
}

char* _cdk_strdup(const char* s) {

	return _strdup(s);
}

void _cdk_sscanf(const char* s, const char* f, va_list v) {

	/**
	 * implementation of vsscanf_s has bugs and will crash, thus using vsscanf instead.
	 */
	vsscanf(s, f, v);
}