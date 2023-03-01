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

_Pragma("once")

#include "cdk/cdk-types.h"

/**
 *  safe strtok, extract sub-string from strings.
 *
 *  @param s   [in] buffer.
 *  @param d   [in] delim.
 *  @param c   [in/out] saved remaining substrings.
 *  @return a pointer to the next sub-string, or NULL if there are no more sub-strings.
 */
	extern char* cdk_strtok(char* s, const char* d, char** c);

/**
 *  concatenate two strings.
 *
 *  @param d   [in/out] dest string buffer.
 *  @param n   [in] dest string buffer size.
 *  @param s   [in] src string buffer.
 *  @return N/A.
 */
extern void cdk_strcat(char* d, size_t n, const char* s);

/**
 *  duplicate a string.
 *
 *  @param s   [in] src string buffer.
 *  @return a pointer to the duplicated string.
 */
extern char* cdk_strdup(const char* s);