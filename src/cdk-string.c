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

#include "platform-string.h"
#include <string.h>
#include <stdarg.h>

int cdk_string_sprintf(char* s, size_t sz, const char* f, ...) {

	int     r;
	va_list v;
	va_start(v, f);
	r = platform_string_sprintf(s, sz, f, v);
	va_end(v);

	return r;
}

int cdk_string_vsprintf(char* s, size_t sz, const char* f, va_list v) {

	return platform_string_sprintf(s, sz, f, v);
}

void cdk_string_sscanf(const char* s, const char* f, ...) {

	va_list v;
	va_start(v, f);
	platform_string_sscanf(s, f, v);
	va_end(v);
}

char* cdk_string_strtok(char* s, const char* d, char** c) {

	return platform_string_strtok(s, d, c);
}

void cdk_string_strcat(char* d, size_t n, const char* s) {

	platform_string_strcat(d, n, s);
}

char* cdk_string_strdup(const char* s) {

	return platform_string_strdup(s);
}

void cdk_string_replace(char* s, char from, char to) {

    for (int i = 0; i < strlen(s); i++) {
        if (s[i] == from) {
            s[i] = to;
        }
    }
}