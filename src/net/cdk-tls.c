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

#include "cdk-tls.h"
#include "cdk/net/cdk-net.h"
#include "cdk-channel.h"
#include "cdk-unpack.h"
#include "cdk/container/cdk-list.h"
#include "platform/platform-event.h"
#include <openssl/ssl.h>
#include <openssl/err.h>

#define __tls_connect_callback	__tls_connect
#define __tls_accept_callback	__tls_accept

static char* __tls_error2string(int err) {
	static char buffer[512];
	ERR_error_string(err, buffer);
	return buffer;
}

cdk_tls_ctx_t* cdk_tls_ctx_create(cdk_tlsconf_t* tlsconf) {
	OPENSSL_init_ssl(OPENSSL_INIT_SSL_DEFAULT, NULL);

	SSL_CTX* ctx = SSL_CTX_new(TLS_method());
	if (!ctx) {
		return NULL;
	}
	if (tlsconf->cafile && tlsconf->capath) {
		if (!SSL_CTX_load_verify_locations(ctx, tlsconf->cafile, tlsconf->capath)) {
			SSL_CTX_free(ctx);
			return NULL;
		}
	}
	if (!tlsconf->crtfile || !tlsconf->keyfile) {
		SSL_CTX_free(ctx);
		return NULL;
	}
	if (!SSL_CTX_use_certificate_file(ctx, tlsconf->crtfile, SSL_FILETYPE_PEM)) {
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
	if (tlsconf->verifypeer && !tlsconf->cafile && !tlsconf->capath) {
		SSL_CTX_set_default_verify_paths(ctx);
	}
	SSL_CTX_set_mode(ctx, SSL_CTX_get_mode(ctx) | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
	SSL_CTX_set_verify(ctx, tlsconf->verifypeer ? SSL_VERIFY_PEER : SSL_VERIFY_NONE, NULL);
	
	return ctx;
}

void cdk_tls_ctx_destroy(cdk_tls_ctx_t* ctx) {
	if (ctx) {
		SSL_CTX_free((SSL_CTX*)ctx);
	}
}

cdk_tls_t* cdk_tls_create(cdk_tls_ctx_t* ctx) {
	if (!ctx) {
		return NULL;
	}
	SSL* ssl = SSL_new(ctx);
	if (!ssl) {
		return NULL;
	}
	return ssl;
}

void cdk_tls_close(cdk_tls_t* tls) {
	if (tls) {
		SSL_shutdown((SSL*)tls);
	}
}

void cdk_tls_destroy(cdk_tls_t* tls) {
	if (tls) {
		SSL_free((SSL*)tls);
	}
}

static void __tls_connect(void* param) {
	cdk_channel_t* channel = param;

	int ret = SSL_connect((SSL*)channel->tcp.tls);
	if (ret == 1) {
		channel->handler->on_connect(channel);
		return;
	}
	int err = SSL_get_error((SSL*)channel->tcp.tls, ret);
	if ((err == SSL_ERROR_WANT_READ) || (err == SSL_ERROR_WANT_WRITE))
	{
		if (err == SSL_ERROR_WANT_READ) {
			channel->cmd = PLATFORM_EVENT_R;
			cdk_channel_modify(channel);
		}
		if (err == SSL_ERROR_WANT_WRITE) {
			channel->cmd = PLATFORM_EVENT_W;
			cdk_channel_modify(channel);
		}
		cdk_event_t* ev = malloc(sizeof(cdk_event_t));
		if (ev) {
			ev->cb = __tls_connect_callback;
			ev->arg = channel;
			cdk_net_postevent(channel->poller, ev);
		}
	}
	else {
		channel->handler->on_close(channel, __tls_error2string(err));
	}
}

static void __tls_accept(void* param) {
	cdk_channel_t* channel = param;

	int ret = SSL_accept((SSL*)channel->tcp.tls);
	if (ret == 1) {
		channel->handler->on_accept(channel);
		return;
	}
	int err = SSL_get_error((SSL*)channel->tcp.tls, ret);
	if ((err == SSL_ERROR_WANT_READ) || (err == SSL_ERROR_WANT_WRITE))
	{
		if (err == SSL_ERROR_WANT_READ) {
			channel->cmd = PLATFORM_EVENT_R;
			cdk_channel_modify(channel);
		}
		if (err == SSL_ERROR_WANT_WRITE) {
			channel->cmd = PLATFORM_EVENT_W;
			cdk_channel_modify(channel);
		}
		cdk_event_t* ev = malloc(sizeof(cdk_event_t));
		if (ev) {
			ev->cb = __tls_accept_callback;
			ev->arg = channel;
			cdk_net_postevent(channel->poller, ev);
		}
	}
	else {
		channel->handler->on_close(channel, __tls_error2string(err));
	}
}

void cdk_tls_connect(cdk_channel_t* channel) {
	SSL_set_fd((SSL*)channel->tcp.tls, (int)channel->fd);
	__tls_connect(channel);
}

void cdk_tls_accept(cdk_channel_t* channel) {
	SSL_set_fd((SSL*)channel->tcp.tls, (int)channel->fd);
	__tls_accept(channel);
}

void cdk_tls_read(cdk_channel_t* channel) {
	int n = SSL_read((SSL*)channel->tcp.tls, (char*)(channel->tcp.rxbuf.buf) + channel->tcp.rxbuf.off, MAX_IOBUF_SIZE);
	if (n <= 0) {
		int err = SSL_get_error((SSL*)channel->tcp.tls, n);
		if (err == SSL_ERROR_WANT_READ) {
			channel->cmd = PLATFORM_EVENT_R;
			cdk_channel_modify(channel);
			return;
		}
		if (err == SSL_ERROR_WANT_WRITE) {
			channel->cmd = PLATFORM_EVENT_W;
			cdk_channel_modify(channel);
			return;
		}
		channel->handler->on_close(channel, __tls_error2string(err));
		return;
	}
	channel->tcp.rxbuf.off += n;
	cdk_unpack(channel);
}

void cdk_tls_write(cdk_channel_t* channel) {
	while (!cdk_list_empty(&(channel->tcp.txlist))) {
		cdk_txlist_node_t* e = cdk_list_data(cdk_list_head(&(channel->tcp.txlist)), cdk_txlist_node_t, n);

		while (e->off < e->len) {
			int n = SSL_write((SSL*)channel->tcp.tls, e->buf + e->off, (int)(e->len - e->off));
			if (n <= 0) {
				int err = SSL_get_error((SSL*)channel->tcp.tls, n);
				if (err == SSL_ERROR_WANT_READ) {
					channel->cmd = PLATFORM_EVENT_R;
					cdk_channel_modify(channel);
					return;
				}
				if (err == SSL_ERROR_WANT_WRITE) {
					channel->cmd = PLATFORM_EVENT_W;
					cdk_channel_modify(channel);
					return;
				}
				channel->handler->on_close(channel, __tls_error2string(err));
				return;
			}
			e->off += n;
		}
		channel->handler->on_write(channel, e->buf, e->len);
		cdk_list_remove(&(e->n));
		free(e);
		e = NULL;
	}
}