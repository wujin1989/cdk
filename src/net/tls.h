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

extern cdk_tls_ctx_t* tls_ctx_create(cdk_tls_conf_t* conf);
extern cdk_tls_ssl_t* tls_ssl_create(cdk_tls_ctx_t* ctx);
extern cdk_tls_bio_t* tls_bio_create(int fd);
extern void           tls_bio_destroy(cdk_tls_bio_t* bio);
extern void           tls_ctx_destroy(cdk_tls_ctx_t* ctx);
extern void           tls_ssl_destroy(cdk_tls_ssl_t* ssl);
extern const char*    tls_error2string(int err);
extern int            tls_connect(cdk_tls_ssl_t* ssl, int fd, int* error);
extern int            tls_accept(cdk_tls_ssl_t* ssl, int fd, bool tcp, int* error);
extern int  tls_listen(cdk_tls_ssl_t* ssl, int fd, cdk_tls_bio_t* bio, int* error);
extern int  tls_ssl_read(cdk_tls_ssl_t* ssl, void* buf, int size, int* error);
extern int  tls_ssl_write(cdk_tls_ssl_t* ssl, void* buf, int size, int* error);
extern void tls_ssl_sni_set(cdk_tls_ssl_t* ssl, const char* sni);
extern void tls_ctx_sni_set(cdk_tls_ctx_t* ctx);
extern void tls_ctx_alpn_set(
    cdk_tls_ctx_t*       ctx,
    const unsigned char* protos,
    unsigned int         protos_len,
    cdk_channel_side_t   side);
