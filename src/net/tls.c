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

#include "tls.h"
#include <openssl/ssl.h>
#include <openssl/err.h>

static SSL_CTX* _tls_ctx_create(cdk_tlsconf_t* tlsconf) {
	SSL_CTX* ctx = NULL;

	OPENSSL_init_ssl(OPENSSL_INIT_SSL_DEFAULT, NULL);
	ctx = SSL_CTX_new(TLS_method());
	if (!ctx) {
		return NULL;
	}
	if (tlsconf->cafile || tlsconf->capath) {
		if (!SSL_CTX_load_verify_locations(ctx, tlsconf->cafile, tlsconf->capath)) {
			SSL_CTX_free(ctx);
			return NULL;
		}
	} else {
		SSL_CTX_set_default_verify_paths(ctx);
	}
	if (tlsconf->crtfile && tlsconf->keyfile) {
		if (!SSL_CTX_use_certificate_chain_file(ctx, tlsconf->crtfile)) {
			SSL_CTX_free(ctx);
			return NULL;
		}
		if (!SSL_CTX_use_PrivateKey_file(ctx, tlsconf->keyfile, SSL_FILETYPE_PEM)) {
			SSL_CTX_free(ctx);
			return NULL;
		}
		if (!SSL_CTX_check_private_key(ctx)) {
			SSL_CTX_free(ctx);
			return NULL;
		}
	}
	SSL_CTX_set_mode(ctx, SSL_CTX_get_mode(ctx) | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
	SSL_CTX_set_verify(ctx, tlsconf->verifypeer ? SSL_VERIFY_PEER : SSL_VERIFY_NONE, NULL);
	return ctx;
}

static void _tls_ctx_destroy(SSL_CTX* ctx) {
	if (ctx) {
		SSL_CTX_free(ctx);
	}
}

const char* tls_ssl_error2string(int err) {
	static char buffer[512];
	ERR_error_string(err, buffer);
	return buffer;
}

cdk_tls_ctx_t* tls_ctx_create(cdk_tlsconf_t* conf) {
	if (!conf->cafile && !conf->capath && !conf->crtfile) {
		return NULL;
	}
	SSL_CTX* ctx = _tls_ctx_create(conf);
	if (!ctx) {
		return NULL;
	}
	return ctx;
}

void tls_ctx_destroy(cdk_tls_ctx_t* ctx) {
	_tls_ctx_destroy((SSL_CTX*)ctx);
}

cdk_tls_ssl_t* tls_ssl_create(cdk_tls_ctx_t* ctx) {
	SSL* ssl = SSL_new((SSL_CTX*)ctx);
	if (!ssl) {
		return NULL;
	}
	return ssl;
}

void tls_ssl_destroy(cdk_tls_ssl_t* ssl) {
	if (ssl) {
		SSL_shutdown((SSL*)ssl);
	}
	if (ssl) {
		SSL_free((SSL*)ssl);
	}
}

int tls_ssl_connect(cdk_tls_ssl_t* ssl, int fd, int* error) {
	SSL_set_fd((SSL*)ssl, fd);
	
	int ret = SSL_connect((SSL*)ssl);
	if (ret <= 0) {
		int err = SSL_get_error((SSL*)ssl, ret);
		*error = err;
		if ((err == SSL_ERROR_WANT_READ) || (err == SSL_ERROR_WANT_WRITE)) {
			return 0;
		}
		return -1;
	}
	return ret;
}

int tls_ssl_accept(cdk_tls_ssl_t* ssl, int fd, int* error) {
	SSL_set_fd((SSL*)ssl, fd);

	int ret = SSL_accept((SSL*)ssl);
	if (ret <= 0) {
		int err = SSL_get_error((SSL*)ssl, ret);
		*error = err;
		if ((err == SSL_ERROR_WANT_READ) || (err == SSL_ERROR_WANT_WRITE)) {
			return 0;
		}
		return -1;
	}
	return ret;
}

int tls_ssl_read(cdk_tls_ssl_t* ssl, void* buf, int size, int* error) {
	int n = SSL_read((SSL*)ssl, buf, size);
	if (n <= 0) {
		int err = SSL_get_error((SSL*)ssl, n);
		*error = err;
		if ((err == SSL_ERROR_WANT_READ) || (err == SSL_ERROR_WANT_WRITE)) {
			return 0;
		}
		return -1;
	}
	return n;
}

int tls_ssl_write(cdk_tls_ssl_t* ssl, void* buf, int size, int* error) {
	int n = SSL_write((SSL*)ssl, buf, size);
	if (n <= 0) {
		int err = SSL_get_error((SSL*)ssl, n);
		*error = err;
		if ((err == SSL_ERROR_WANT_READ) || (err == SSL_ERROR_WANT_WRITE)) {
			return 0;
		}
		return -1;
	}
	return n;
}