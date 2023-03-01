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

#include <stdio.h>
#include <stdbool.h>

/**
 *  encapsulates fopen, windows with fopen_s, unix with fopen.
 *  note: the path separator is unified with '/'.
 *
 *  @param fp [in/out].
 *  @param f  [in].
 *  @param m  [in].
 *  @return N/A.
 */
extern void cdk_io_fopen(FILE** fp, const char* restrict f, const char* restrict m);

/**
 *  encapsulates fclose.
 *
 *  @param s [in].
 *  @return false means failure, true means success.
 */
extern void cdk_io_fclose(FILE* s);

/**
 *  safe sprintf, write formatted data to a string.
 *
 *  @param s   [in/out] buffer.
 *  @param sz  [in] buffer size.
 *  @param f   [in] specified format.
 *  @param ... [in] variable parameter.
 *  @return the number of bytes written to the buffer, not counting the terminating null character. a negative value if error occurs.
 */
extern int cdk_io_sprintf(char* s, size_t sz, const char* f, ...);

/**
 *  safe vsprintf.
 *
 *  @param s   [in/out] buffer.
 *  @param sz  [in] buffer size.
 *  @param f   [in] specified format.
 *  @param v   [in] variable parameter.
 *  @return the number of bytes written to the buffer, not counting the terminating null character. a negative value if error occurs.
 */
extern int cdk_io_vsprintf(char* s, size_t sz, const char* f, va_list v);

/**
 *  reads formatted data from a string.
 *
 *  @param s   [in] buffer.
 *  @param f   [in] specified format.
 *  @param ... [in/out] variable parameter.
 *  @return N/A.
 */
extern void cdk_io_sscanf(const char* s, const char* f, ...);


