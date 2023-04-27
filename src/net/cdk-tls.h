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

_Pragma("once")

#include "cdk/cdk-types.h"

extern cdk_tls_ctx_t* cdk_tls_ctx_create(cdk_tlsconf_t* tlsconf);
extern void           cdk_tls_ctx_destroy(cdk_tls_ctx_t* ctx);
extern cdk_tls_t*     cdk_tls_create(cdk_tls_ctx_t* ctx);
extern void           cdk_tls_close(cdk_tls_t* secure);
extern void           cdk_tls_destroy(cdk_tls_t* secure);
extern void           cdk_tls_connect(cdk_channel_t* channel);
extern void           cdk_tls_accept(cdk_channel_t* channel);
extern void           cdk_tls_read(cdk_channel_t* channel);
extern void           cdk_tls_write(cdk_channel_t* channel);
