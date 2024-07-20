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

typedef enum ctrl_side_e {
	CTRL_CLIENT,
	CTRL_SERVER,
} ctrl_side_t;

extern cdk_tls_ctx_t* tls_ctx_create(cdk_tls_conf_t* conf);
extern cdk_tls_ssl_t* tls_ssl_create(cdk_tls_ctx_t* ctx);
extern const char* tls_ssl_error2string(int err);
extern void tls_ctx_destroy(cdk_tls_ctx_t* ctx);
extern void tls_ssl_destroy(cdk_tls_ssl_t* ssl);
extern int tls_ssl_connect(cdk_tls_ssl_t* ssl, int fd, int* error);
extern int tls_ssl_accept(cdk_tls_ssl_t* ssl, int fd, int* error);
extern int tls_ssl_read(cdk_tls_ssl_t* ssl, void* buf, int size, int* error);
extern int tls_ssl_write(cdk_tls_ssl_t* ssl, void* buf, int size, int* error);
extern void tls_ssl_sni_set(cdk_tls_ssl_t* ssl, const char* sni);
extern void tls_ctx_sni_set(cdk_tls_ctx_t* ctx);
extern void tls_ctx_alpn_set(cdk_tls_ctx_t* ctx, const unsigned char* protos, unsigned int protos_len, ctrl_side_t side);

