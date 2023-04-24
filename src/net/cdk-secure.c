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

#include "cdk-secure.h"
#include <openssl/ssl.h>

//
//SSL_set_fd(ssl, 1);
//
//SSL_accept(ssl);
//SSL_connect(ssl);
//SSL_read(ssl, NULL, 0);
//int ret = SSL_write(ssl, NULL, 0);
//SSL_shutdown(ssl);
//int err = SSL_get_error(ssl, ret);
//if (err == SSL_ERROR_WANT_READ) {}

cdk_ssl_ctx_t* cdk_secure_ctxcreate(const char* cafile, const char* capath, const char* crtfile, const char* keyfile) {
	OPENSSL_init_ssl(OPENSSL_INIT_SSL_DEFAULT, NULL);
	SSL_CTX* ctx = SSL_CTX_new(TLS_method());
	if (!ctx) {
		return NULL;
	}
	if (cafile && capath) {
		if (!SSL_CTX_load_verify_locations(ctx, cafile, capath)) {
			SSL_CTX_free(ctx);
			return NULL;
		}
	}
	if (!crtfile || !keyfile) {
		SSL_CTX_free(ctx);
		return NULL;
	}
	if (!SSL_CTX_use_certificate_file(ctx, crtfile, SSL_FILETYPE_PEM)) {
		SSL_CTX_free(ctx);
		return NULL;
	}
	if (!SSL_CTX_use_PrivateKey_file(ctx, keyfile, SSL_FILETYPE_PEM)) {
		SSL_CTX_free(ctx);
		return NULL;
	}
	if (!SSL_CTX_check_private_key(ctx)) {
		SSL_CTX_free(ctx);
		return NULL;
	}
	SSL_CTX_set_default_verify_paths(ctx);
	SSL_CTX_set_mode(ctx, SSL_CTX_get_mode(ctx) | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);

	return ctx;
}

void cdk_secure_ctxdestroy(cdk_ssl_ctx_t* ctx) {
	if (ctx) {
		SSL_CTX_free((SSL_CTX*)ctx);
	}
}

cdk_ssl_t* cdk_secure_sslcreate(cdk_ssl_ctx_t* ctx) {
	SSL* ssl = SSL_new(ctx);
	if (!ssl) {
		return NULL;
	}
	return ssl;
}

void cdk_secure_ssldestroy(cdk_ssl_t* ssl) {
	if (ssl) {
		SSL_free(ssl);
	}
}