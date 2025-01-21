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
#include "cdk/net/cdk-net.h"
#include "platform/platform-socket.h"
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>

#define COOKIE_SECRET_LENGTH 16

static unsigned char cookie_secret[COOKIE_SECRET_LENGTH];
static bool          cookie_initialized;

static int _alpn_select_cb(
    SSL*                  ssl,
    const unsigned char** out,
    unsigned char*        outlen,
    const unsigned char*  in,
    unsigned int          inlen,
    void*                 arg) {

    return 0;
}

static int _sni_select_cb(SSL* s, int* al, void* arg) {
    return 0;
}

static int
_gen_cookie_cb(SSL* ssl, unsigned char* cookie, unsigned int* cookie_len) {
    unsigned char *buffer, result[EVP_MAX_MD_SIZE];
    unsigned int   length = 0, resultlength;
    union {
        struct sockaddr_storage ss;
        struct sockaddr_in6     s6;
        struct sockaddr_in      s4;
    } peer;

    /* Initialize a random secret */
    if (!cookie_initialized) {
        if (!RAND_bytes(cookie_secret, COOKIE_SECRET_LENGTH)) {
            return 0;
        }
        cookie_initialized = 1;
    }

    /* Read peer information */
    (void)BIO_dgram_get_peer(SSL_get_rbio(ssl), &peer);

    /* Create buffer with peer's address and port */
    length = 0;
    switch (peer.ss.ss_family) {
    case AF_INET:
        length += sizeof(struct in_addr);
        break;
    case AF_INET6:
        length += sizeof(struct in6_addr);
        break;
    default:
        OPENSSL_assert(0);
        break;
    }
    length += 2;
    buffer = (unsigned char*)OPENSSL_malloc(length);

    if (buffer == NULL) {
        return 0;
    }

    switch (peer.ss.ss_family) {
    case AF_INET:
        memcpy(buffer, &peer.s4.sin_port, 2);
        memcpy(
            buffer + sizeof(peer.s4.sin_port),
            &peer.s4.sin_addr,
            sizeof(struct in_addr));
        break;
    case AF_INET6:
        memcpy(buffer, &peer.s6.sin6_port, 2);
        memcpy(buffer + 2, &peer.s6.sin6_addr, sizeof(struct in6_addr));
        break;
    default:
        OPENSSL_assert(0);
        break;
    }

    /* Calculate HMAC of buffer using the secret */
    HMAC(
        EVP_sha1(),
        (const void*)cookie_secret,
        COOKIE_SECRET_LENGTH,
        (const unsigned char*)buffer,
        length,
        result,
        &resultlength);
    OPENSSL_free(buffer);

    memcpy(cookie, result, resultlength);
    *cookie_len = resultlength;

    return 1;
}

static int _verify_cookie_cb(
    SSL* ssl, const unsigned char* cookie, unsigned int cookie_len) {
    unsigned char result[EVP_MAX_MD_SIZE];
    unsigned int  resultlength;

    if (cookie_initialized && _gen_cookie_cb(ssl, result, &resultlength) &&
        cookie_len == resultlength && !memcmp(result, cookie, resultlength)) {
        return 1;
    }
    return 0;
}

static SSL_CTX* _ctx_create(cdk_tls_conf_t* tlsconf) {
    SSL_CTX* ctx = NULL;

    OPENSSL_init_ssl(OPENSSL_INIT_SSL_DEFAULT, NULL);
    if (tlsconf->dtls) {
        ctx = SSL_CTX_new(DTLS_method());
    } else {
        ctx = SSL_CTX_new(TLS_method());
    }
    if (!ctx) {
        return NULL;
    }
    if (tlsconf->cafile || tlsconf->capath) {
        if (!SSL_CTX_load_verify_locations(
                ctx, tlsconf->cafile, tlsconf->capath)) {
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
        if (!SSL_CTX_use_PrivateKey_file(
                ctx, tlsconf->keyfile, SSL_FILETYPE_PEM)) {
            SSL_CTX_free(ctx);
            return NULL;
        }
        if (!SSL_CTX_check_private_key(ctx)) {
            SSL_CTX_free(ctx);
            return NULL;
        }
    }
    /**
     * If the mode is not enabled, when calling SSL_write results in only
     * partial data being written, the remaining data will be copied into cdk's
     * internal tx queue. When attempting to write more data by calling
     * SSL_write again, OpenSSL will verify if the buffer address for this write
     * operation matches the one used in the previous incomplete write
     * operation. If the addresses do not match, OpenSSL will return an error,
     * potentially leading to the closure of the connection.
     * By enabling the mode, OpenSSL permits the use of different buffers for
     * successive write operations, but the caller is still responsible for
     * ensuring the consistency of the data.
     */
    SSL_CTX_set_mode(
        ctx, SSL_CTX_get_mode(ctx) | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
    SSL_CTX_set_verify(
        ctx,
        tlsconf->verifypeer ? SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT
                            : SSL_VERIFY_NONE,
        NULL);

    if (tlsconf->side == CHANNEL_SIDE_SERVER) {
        if (tlsconf->dtls) {
            SSL_CTX_set_cookie_generate_cb(ctx, _gen_cookie_cb);
            SSL_CTX_set_cookie_verify_cb(ctx, _verify_cookie_cb);
        }
    }
    return ctx;
}

const char* tls_error2string(int err) {
    static char buffer[512];
    const char* error_str = "Unknown SSL error code";
    const char* sys_error_str = NULL;

    memset(buffer, 0, sizeof(buffer));

    switch (err) {
    case SSL_ERROR_NONE:
        error_str = "SSL_ERROR_NONE: No error";
        break;
    case SSL_ERROR_ZERO_RETURN:
        error_str = "SSL_ERROR_ZERO_RETURN: Connection closed cleanly";
        break;
    case SSL_ERROR_WANT_READ:
        error_str = "SSL_ERROR_WANT_READ: Operation needs to read more data";
        break;
    case SSL_ERROR_WANT_WRITE:
        error_str = "SSL_ERROR_WANT_WRITE: Operation needs to write more data";
        break;
    case SSL_ERROR_WANT_CONNECT:
        error_str =
            "SSL_ERROR_WANT_CONNECT: Non-blocking connect() in progress";
        break;
    case SSL_ERROR_WANT_ACCEPT:
        error_str = "SSL_ERROR_WANT_ACCEPT: Non-blocking accept() in progress";
        break;
    case SSL_ERROR_WANT_X509_LOOKUP:
        error_str = "SSL_ERROR_WANT_X509_LOOKUP: Operation needs X.509 certificate lookup";
        break;
    case SSL_ERROR_WANT_ASYNC:
        error_str = "SSL_ERROR_WANT_ASYNC: Async operation in progress";
        break;
    case SSL_ERROR_WANT_ASYNC_JOB:
        error_str = "SSL_ERROR_WANT_ASYNC_JOB: Async job in progress";
        break;
    case SSL_ERROR_WANT_CLIENT_HELLO_CB:
        error_str =
            "SSL_ERROR_WANT_CLIENT_HELLO_CB: Client Hello callback in progress";
        break;
    case SSL_ERROR_SYSCALL:
        error_str = "SSL_ERROR_SYSCALL: A system call failed";
        sys_error_str =
            platform_socket_error2string(platform_socket_lasterror());
        if (sys_error_str != NULL) {
            snprintf(
                buffer, sizeof(buffer), "%s (%s)", error_str, sys_error_str);
            return buffer;
        }
        break;
    case SSL_ERROR_SSL:
        error_str = "SSL_ERROR_SSL: An SSL library error occurred";
        break;
    default:
        break;
    }
    memcpy(buffer, error_str, strlen(error_str));
    return buffer;
}

cdk_tls_ctx_t* tls_ctx_create(cdk_tls_conf_t* conf) {
    if (!conf) {
        return NULL;
    }
    if (!conf->cafile && !conf->capath && !conf->crtfile) {
        return NULL;
    }
    SSL_CTX* ctx = _ctx_create(conf);
    if (!ctx) {
        return NULL;
    }
    return ctx;
}

void tls_ctx_destroy(cdk_tls_ctx_t* ctx) {
    if ((SSL_CTX*)ctx) {
        SSL_CTX_free((SSL_CTX*)ctx);
        ctx = NULL;
    }
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
        SSL_free((SSL*)ssl);
        ssl = NULL;
    }
}

cdk_tls_bio_t* tls_bio_create(int fd) {
    return BIO_new_dgram(fd, 0);
}

void tls_bio_destroy(cdk_tls_bio_t* bio) {
    BIO_free((BIO*)bio);
}

int tls_connect(cdk_tls_ssl_t* ssl, int fd, int* error) {
    /**
     * For the client, since we use already connected sockets for both TLS and
     * DTLS, we should use the TLS method SSL_set_fd instead of the
     * DTLS-specific SSL_set_bio((SSL*)ssl, BIO_new_dgram(fd, 0), bio)
     */
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

int tls_accept(cdk_tls_ssl_t* ssl, int fd, bool tcp, int* error) {
    if (tcp) {
        SSL_set_fd((SSL*)ssl, fd);
    }
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

int tls_listen(cdk_tls_ssl_t* ssl, int fd, cdk_tls_bio_t* bio, int* error) {
    /**
     * For the server, UDP sockets cannot use already connected sockets because
     * they need to send data to multiple peers. Therefore, we cannot use
     * SSL_set_fd, as SSL_set_fd is only suitable for already connected sockets.
     */
    SSL_set_bio((SSL*)ssl, (BIO*)bio, bio);

    BIO_ADDR* noused = BIO_ADDR_new();
    int       ret = DTLSv1_listen((SSL*)ssl, noused);
    if (ret <= 0) {
        int err = SSL_get_error((SSL*)ssl, ret);
        *error = err;
        BIO_ADDR_free(noused);
        return ret;
    }
    BIO_ADDR_free(noused);
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

void tls_ssl_sni_set(cdk_tls_ssl_t* ssl, const char* sni) {
    /* used by client side, support tls, dtls */
    SSL_set_tlsext_host_name((SSL*)ssl, sni);
}

void tls_ctx_sni_set(cdk_tls_ctx_t* ctx) {
    /* used by server side, support tls, dtls */
    SSL_CTX_set_tlsext_servername_callback(ctx, _sni_select_cb);
}

void tls_ctx_alpn_set(
    cdk_tls_ctx_t*       ctx,
    const unsigned char* protos,
    unsigned int         protos_len,
    cdk_channel_side_t   side) {
    /* used by dual side, support tls, dtls */
    if (side == CHANNEL_SIDE_CLIENT) {
        SSL_CTX_set_alpn_protos((SSL_CTX*)ctx, protos, protos_len);
    }
    if (side == CHANNEL_SIDE_SERVER) {
        SSL_CTX_set_alpn_select_cb(
            (SSL_CTX*)ctx, _alpn_select_cb, (void*)protos);
    }
}
