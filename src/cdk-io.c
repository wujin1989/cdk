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

#include "cdk/cdk-io.h"
#include <stdarg.h>

#if defined(__linux__) || defined(__APPLE__)
#include "unix/unix-io.h"
#endif

#if defined(_WIN32)
#include "win/win-io.h"
#endif

void cdk_fopen(FILE** fp, const char* restrict f, const char* restrict m) {
	
	_cdk_fopen(fp, f, m);
}

bool cdk_fclose(FILE* s) {

	int r;
	if (s) {
		r = fclose(s);
		if (r == EOF) { return false; }
		s = NULL;
	}
	return true;
}

int cdk_sprintf(char* s, size_t sz, const char* f, ...) {

	int     r;
	va_list v;
	va_start(v, f);
	r = _cdk_sprintf(s, sz, f, v);
	va_end(v);

	return r;
}

int cdk_vsprintf(char* s, size_t sz, const char* f, va_list v) {

	return _cdk_sprintf(s, sz, f, v);
}

char* cdk_strtok(char* s, const char* d, char** c) {

	return _cdk_strtok(s, d, c);
}

void cdk_strcat(char* d, size_t n, const char* s) {

	_cdk_strcat(d, n, s);
}

char* cdk_strdup(const char* s) {

	return _cdk_strdup(s);
}

void cdk_sscanf(const char* s, const char* f, ...) {

	va_list v;
	va_start(v, f);
	_cdk_sscanf(s, f, v);
	va_end(v);
}