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

#include "platform/platform-socket.h"
#include "platform/platform-event.h"
#include "cdk/container/cdk-list.h"
#include "cdk-channel.h"
#include "cdk-unpack.h"
#include "cdk-secure.h"
#include "cdk/cdk-timer.h"
#include "cdk/cdk-time.h"
#include "cdk/net/cdk-net.h"
#include "cdk/container/cdk-rbtree.h"

extern cdk_tls_ctx_t* tlsctx;
extern cdk_dtls_ctx_t* dtlsctx;
extern cdk_timer_t timer;
extern cdk_poller_t* _poller_roundrobin(void);

static void __channel_destroy_callback(void* param) {
    cdk_channel_t* channel = param;

    mtx_destroy(&channel->mtx);
    free(channel);
    channel = NULL;
}

static void __channel_tcprecv(cdk_channel_t* channel) {
    ssize_t n = platform_socket_recv(channel->fd, (char*)(channel->tcp.rxbuf.buf) + channel->tcp.rxbuf.off, MAX_IOBUF_SIZE);
    if (n == -1) {
        if ((platform_socket_lasterror() != PLATFORM_SO_ERROR_EAGAIN)
            || (platform_socket_lasterror() != PLATFORM_SO_ERROR_WSAEWOULDBLOCK)) {
            channel->handler->on_close(channel, platform_socket_error2string(platform_socket_lasterror()));
        }
        return;
    }
    /**
     * windows provides the WSAECONNRESET to detect a peer disconnection,
     * so it is not necessary to rely on receiving 0 from recv to determine that the connection has been closed.
     * but it is necessary to unix.
     */
    if (n == 0) {
        channel->handler->on_close(channel, platform_socket_error2string(PLATFORM_SO_ERROR_WSAECONNRESET));
        return;
    }
    channel->tcp.rxbuf.off += n;
    cdk_unpack(channel);
}

static void __channel_udprecv(cdk_channel_t* channel) {
    channel->udp.peer.sslen = sizeof(struct sockaddr_storage);
    ssize_t n = platform_socket_recvfrom(channel->fd, channel->udp.rxbuf.buf, MAX_IOBUF_SIZE, &channel->udp.peer.ss, &channel->udp.peer.sslen);
    if (n == -1) {
        /**
         * to be compatible with the semantics of unix, windows's WSAECONNRESET is filtered out.
         */
        if (((platform_socket_lasterror() != PLATFORM_SO_ERROR_EAGAIN)
            || (platform_socket_lasterror() != PLATFORM_SO_ERROR_WSAEWOULDBLOCK))
            && (platform_socket_lasterror() != PLATFORM_SO_ERROR_WSAECONNRESET)) {
            channel->handler->on_close(channel, platform_socket_error2string(platform_socket_lasterror()));
        }
        return;
    }
    channel->handler->on_read(channel, channel->udp.rxbuf.buf, n);
}

static void __channel_tcpsend(cdk_channel_t* channel) {
    while (!cdk_list_empty(&(channel->tcp.txlist))) {
        cdk_txlist_node_t* e = cdk_list_data(cdk_list_head(&(channel->tcp.txlist)), cdk_txlist_node_t, n);

        while (e->off < e->len) {
            ssize_t n = platform_socket_send(channel->fd, e->buf + e->off, (int)(e->len - e->off));
            if (n == -1) {
                if ((platform_socket_lasterror() != PLATFORM_SO_ERROR_EAGAIN)
                    || (platform_socket_lasterror() != PLATFORM_SO_ERROR_WSAEWOULDBLOCK)) {
                    channel->handler->on_close(channel, platform_socket_error2string(platform_socket_lasterror()));
                }
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

static void __channel_udpsend(cdk_channel_t* channel) {
    while (!cdk_list_empty(&(channel->udp.txlist))) {
        cdk_txlist_node_t* e = cdk_list_data(cdk_list_head(&(channel->udp.txlist)), cdk_txlist_node_t, n);

        ssize_t n = platform_socket_sendto(channel->fd, e->buf, (int)e->len, &(channel->udp.peer.ss), channel->udp.peer.sslen);
        if (n == -1) {
            if ((platform_socket_lasterror() != PLATFORM_SO_ERROR_EAGAIN)
                || (platform_socket_lasterror() != PLATFORM_SO_ERROR_WSAEWOULDBLOCK)) {
                channel->handler->on_close(channel, platform_socket_error2string(platform_socket_lasterror()));
            }
            return;
        }
        channel->handler->on_write(channel, e->buf, e->len);
        cdk_list_remove(&(e->n));
        free(e);
        e = NULL;
    }
}

static void __channel_tcpaccept(cdk_channel_t* channel) {
    cdk_sock_t cli = platform_socket_accept(channel->fd);
    if (cli == -1) {
        if ((platform_socket_lasterror() != PLATFORM_SO_ERROR_EAGAIN)
            || (platform_socket_lasterror() != PLATFORM_SO_ERROR_WSAEWOULDBLOCK)) {
            channel->handler->on_close(channel, platform_socket_error2string(platform_socket_lasterror()));
        }
        return;
    }
    cdk_channel_t* newchannel = cdk_channel_create(_poller_roundrobin(), cli, PLATFORM_EVENT_R, channel->handler);
    channel->handler->on_accept(newchannel);
}

cdk_channel_t* cdk_channel_create(cdk_poller_t* poller, cdk_sock_t sock, int cmd, cdk_handler_t* handler)
{
    cdk_channel_t* channel = malloc(sizeof(cdk_channel_t));

    if (channel) {
        memset(channel, 0, sizeof(cdk_channel_t));
        channel->poller = poller;
        channel->cmd = cmd;
        channel->fd = sock;
        channel->handler = handler;
        channel->type = platform_socket_socktype(sock);
        channel->active = true;
        mtx_init(&channel->mtx, mtx_plain);

        if (channel->type == SOCK_STREAM) {
            channel->tcp.rxbuf.len = MAX_IOBUF_SIZE;
            channel->tcp.rxbuf.off = 0;
            channel->tcp.rxbuf.buf = malloc(MAX_IOBUF_SIZE);
            if (channel->tcp.rxbuf.buf) {
                memset(channel->tcp.rxbuf.buf, 0, MAX_IOBUF_SIZE);
            }
            cdk_list_init(&(channel->tcp.txlist));
            channel->tcp.tls = cdk_secure_tls_create(tlsctx);
        }
        if (channel->type == SOCK_DGRAM) {
            channel->udp.rxbuf.len = MAX_IOBUF_SIZE;
            channel->udp.rxbuf.off = 0;
            channel->udp.rxbuf.buf = malloc(MAX_IOBUF_SIZE);
            if (channel->udp.rxbuf.buf) {
                memset(channel->udp.rxbuf.buf, 0, MAX_IOBUF_SIZE);
            }
            cdk_list_init(&(channel->udp.txlist));
            channel->udp.dtls = cdk_secure_dtls_create(dtlsctx);
        }
        platform_event_add(poller->pfd, channel->fd, cmd, channel);
        return channel;
    }
    return NULL;
}

void cdk_channel_modify(cdk_channel_t* channel) {
    platform_event_mod(channel->poller->pfd, channel->fd, channel->cmd, channel);
}

void cdk_channel_destroy(cdk_channel_t* channel)
{
    mtx_lock(&channel->mtx);
    channel->active = false;

    platform_event_del(channel->poller->pfd, channel->fd);
    platform_socket_close(channel->fd);

    if (channel->type == SOCK_STREAM) {
        free(channel->tcp.rxbuf.buf);
        channel->tcp.rxbuf.buf = NULL;

        while (!cdk_list_empty(&(channel->tcp.txlist))) {
            cdk_txlist_node_t* e = cdk_list_data(cdk_list_head(&(channel->tcp.txlist)), cdk_txlist_node_t, n);
            cdk_list_remove(&(e->n));
            free(e);
            e = NULL;
        }
        cdk_secure_tls_destroy(channel->tcp.tls);
    }
    if (channel->type == SOCK_DGRAM) {
        free(channel->udp.rxbuf.buf);
        channel->udp.rxbuf.buf = NULL;

        while (!cdk_list_empty(&(channel->udp.txlist))) {
            cdk_txlist_node_t* e = cdk_list_data(cdk_list_head(&(channel->udp.txlist)), cdk_txlist_node_t, n);
            cdk_list_remove(&(e->n));
            free(e);
            e = NULL;
        }
        cdk_secure_dtls_destroy(channel->udp.dtls);
    }
    mtx_unlock(&channel->mtx);

    cdk_timer_add(&timer, __channel_destroy_callback, channel, 10000, false);
}

void cdk_channel_recv(cdk_channel_t* channel) {
    if (channel->type == SOCK_STREAM) {
        __channel_tcprecv(channel);
    }
    if (channel->type == SOCK_DGRAM) {
        __channel_udprecv(channel);
    }
}

void cdk_channel_send(cdk_channel_t* channel) {
    if (channel->type == SOCK_STREAM) {
        __channel_tcpsend(channel);
    }
    if (channel->type == SOCK_DGRAM) {
        __channel_udpsend(channel);
    }
}

void cdk_channel_accept(cdk_channel_t* channel) {
    __channel_tcpaccept(channel);
}

void cdk_channel_connect(cdk_channel_t* channel) {
    int err;
    socklen_t len;
    len = sizeof(int);

    cdk_timer_del(&timer, channel->tcp.ctimer);

    getsockopt(channel->fd, SOL_SOCKET, SO_ERROR, (char*)&err, &len);
    if (err) {
        channel->handler->on_close(channel, platform_socket_error2string(err));
    }
    else {
        if (tlsctx) {
            cdk_secure_tls_connect(channel);
        }
        else {
            channel->handler->on_connect(channel);
        }
    }
}