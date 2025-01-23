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

#include "channel.h"
#include "cdk/cdk-time.h"
#include "cdk/cdk-timer.h"
#include "cdk/container/cdk-list.h"
#include "cdk/container/cdk-rbtree.h"
#include "cdk/net/cdk-net.h"
#include "platform/platform-event.h"
#include "platform/platform-socket.h"
#include "tls.h"
#include "txlist.h"
#include "unpacker.h"

#define CHANNEL_DELAYED_DESTROY_TIME 1000 * 60  //1min

extern cdk_net_engine_t global_net_engine;

static inline void _write_complete_cb(void* param) {
    cdk_channel_t* channel = param;

    if (channel->type == SOCK_STREAM) {
        channel->handler->tcp.on_write(channel);
    }
    if (channel->type == SOCK_DGRAM) {
        channel->handler->udp.on_write(channel);
    }
}

static void _tcp_send(cdk_channel_t* channel) {
    if (txlist_empty(&channel->txlist)) {
        if (channel_is_writing(channel)) {
            channel_disable_write(channel);
        }
        return;
    }
    txlist_node_t* e =
        cdk_list_data(cdk_list_head(&(channel->txlist)), txlist_node_t, n);

    ssize_t n = platform_socket_send(channel->fd, e->buf, (int)e->len);
    if (n == PLATFORM_SO_ERROR_SOCKET_ERROR) {
        if ((platform_socket_lasterror() == PLATFORM_SO_ERROR_EAGAIN) ||
            (platform_socket_lasterror() == PLATFORM_SO_ERROR_EWOULDBLOCK)) {
            return;
        }
        channel_destroy(
            channel,
            CHANNEL_REASON_SYSCALL_FAIL,
            platform_socket_error2string(platform_socket_lasterror()));
        return;
    }
    if (n < e->len) {
        txlist_insert(&channel->txlist, e->buf + n, (e->len - n), false);
    }
    channel->tcp.latest_wr_time = cdk_time_now();
    if (n > 0 && channel->handler->tcp.on_write) {
        channel->handler->tcp.on_write(channel);
    }
    txlist_remove(e);
}

static void _udp_send(cdk_channel_t* channel) {
    if (txlist_empty(&channel->txlist)) {
        if (channel_is_writing(channel)) {
            channel_disable_write(channel);
        }
        return;
    }
    ssize_t        n = 0;
    txlist_node_t* e =
        cdk_list_data(cdk_list_head(&(channel->txlist)), txlist_node_t, n);

    if (channel->side == SIDE_CLIENT) {
        n = platform_socket_send(channel->fd, e->buf, (int)e->len);
    }
    if (channel->side == SIDE_SERVER) {
        n = platform_socket_sendto(
            channel->fd,
            e->buf,
            (int)e->len,
            &(channel->udp.peer.ss),
            channel->udp.peer.sslen);
    }
    if (n == PLATFORM_SO_ERROR_SOCKET_ERROR) {
        if ((platform_socket_lasterror() == PLATFORM_SO_ERROR_EAGAIN) ||
            (platform_socket_lasterror() == PLATFORM_SO_ERROR_EWOULDBLOCK)) {
            return;
        }
        channel_destroy(
            channel,
            CHANNEL_REASON_SYSCALL_FAIL,
            platform_socket_error2string(platform_socket_lasterror()));
        return;
    }
    if (n < e->len) {
        txlist_insert(&channel->txlist, e->buf + n, (e->len - n), false);
    }
    if (n > 0 && channel->handler->udp.on_write) {
        channel->handler->udp.on_write(channel);
    }
    txlist_remove(e);
}

static void _tls_send(cdk_channel_t* channel) {
    int err = 0;
    if (txlist_empty(&channel->txlist)) {
        if (channel_is_writing(channel)) {
            channel_disable_write(channel);
        }
        return;
    }
    txlist_node_t* e =
        cdk_list_data(cdk_list_head(&(channel->txlist)), txlist_node_t, n);

    int n = tls_ssl_write(channel->tcp.tls_ssl, e->buf, (int)e->len, &err);
    if (n <= 0) {
        if (n == 0) {
            return;
        }
        channel_destroy(
            channel, CHANNEL_REASON_TLS_FAIL, tls_error2string(err));
        return;
    }
    if (n < e->len) {
        txlist_insert(&channel->txlist, e->buf + n, (e->len - n), false);
    }
    channel->tcp.latest_wr_time = cdk_time_now();
    if (n > 0 && channel->handler->tcp.on_write) {
        channel->handler->tcp.on_write(channel);
    }
    txlist_remove(e);
}

// static void _dtls_send(cdk_channel_t* channel) {
//     int err = 0;
//     if (txlist_empty(&channel->txlist)) {
//         if (channel_is_writing(channel)) {
//             channel_disable_write(channel);
//         }
//         return;
//     }
//     txlist_node_t* e =
//         cdk_list_data(cdk_list_head(&(channel->txlist)), txlist_node_t, n);
//     int n = tls_ssl_write(
//         (channel->type == SOCK_STREAM) ? channel->tcp.ssl
//                                        : channel->udp.ssl->dtls_ssl,
//         e->buf,
//         (int)e->len,
//         &err);
//     if (n <= 0) {
//         if (n == 0) {
//             return;
//         }
//         channel_destroy(channel, CHANNEL_REASON_TLS_FAIL,
//         tls_error2string(err)); return;
//     }
//     if (n < e->len) {
//         txlist_insert(&channel->txlist, e->buf + n, (e->len - n), false);
//     }
//     if (channel->type == SOCK_STREAM) {
//         channel->tcp.latest_wr_time = cdk_time_now();
//         if (channel->handler->tcp.on_write) {
//             channel->handler->tcp.on_write(channel);
//         }
//     }
//     if (channel->type == SOCK_DGRAM) {
//         if (channel->handler->udp.on_write) {
//             channel->handler->udp.on_write(channel);
//         }
//     }
//     txlist_remove(e);
// }

static void
_tls_explicit_send(cdk_channel_t* channel, void* data, size_t size) {
    int n = 0;
    if (txlist_empty(&channel->txlist)) {
        int err = 0;
        n = tls_ssl_write(channel->tcp.tls_ssl, data, size, &err);
        if (n <= 0) {
            if (n == 0) {
                txlist_insert(&channel->txlist, data, size, true);
                if (!channel_is_writing(channel)) {
                    channel_enable_write(channel);
                }
                return;
            }
            channel_destroy(
                channel, CHANNEL_REASON_TLS_FAIL, tls_error2string(err));
            return;
        }
    }
    if (n < size) {
        txlist_insert(&channel->txlist, (char*)data + n, (size - n), true);
        if (!channel_is_writing(channel)) {
            channel_enable_write(channel);
        }
    }
    channel->tcp.latest_wr_time = cdk_time_now();
    if (n > 0 && channel->handler->tcp.on_write) {
        cdk_net_post_event(channel->poller, _write_complete_cb, channel, true);
    }
}

// static void _dtls_explicit_send(cdk_channel_t* channel, void* data, size_t
// size) {
//     int n = 0;
//     if (txlist_empty(&channel->txlist)) {
//         int err = 0;
//         n = tls_ssl_write(
//             (channel->type == SOCK_STREAM) ? channel->tcp.ssl
//                                            : channel->udp.ssl->dtls_ssl,
//             data,
//             size,
//             &err);
//         if (n <= 0) {
//             if (n == 0) {
//                 txlist_insert(&channel->txlist, data, size, true);
//                 if (!channel_is_writing(channel)) {
//                     channel_enable_write(channel);
//                 }
//                 return;
//             }
//             channel_destroy(channel, CHANNEL_REASON_TLS_FAIL,
//             tls_error2string(err)); return;
//         }
//     }
//     if (n < size) {
//         txlist_insert(&channel->txlist, (char*)data + n, (size - n), true);
//         if (!channel_is_writing(channel)) {
//             channel_enable_write(channel);
//         }
//     }
//     if (channel->type == SOCK_STREAM) {
//         channel->tcp.latest_wr_time = cdk_time_now();
//         if (n > 0 && channel->handler->tcp.on_write) {
//             cdk_net_postevent(
//                 channel->poller, _cb_write_complete, channel, true);
//         }
//     }
//     if (channel->type == SOCK_DGRAM) {
//         if (n > 0 && channel->handler->udp.on_write) {
//             cdk_net_postevent(
//                 channel->poller, _cb_write_complete, channel, true);
//         }
//     }
// }

static void
_tcp_explicit_send(cdk_channel_t* channel, void* data, size_t size) {
    ssize_t n = 0;
    if (txlist_empty(&channel->txlist)) {
        n = platform_socket_send(channel->fd, data, size);
        if (n == PLATFORM_SO_ERROR_SOCKET_ERROR) {
            if ((platform_socket_lasterror() == PLATFORM_SO_ERROR_EAGAIN) ||
                (platform_socket_lasterror() ==
                 PLATFORM_SO_ERROR_EWOULDBLOCK)) {

                txlist_insert(&channel->txlist, data, size, true);
                if (!channel_is_writing(channel)) {
                    channel_enable_write(channel);
                }
                return;
            }
            channel_destroy(
                channel,
                CHANNEL_REASON_SYSCALL_FAIL,
                platform_socket_error2string(platform_socket_lasterror()));
            return;
        }
    }
    if (n < size) {
        txlist_insert(&channel->txlist, (char*)data + n, (size - n), true);
        if (!channel_is_writing(channel)) {
            channel_enable_write(channel);
        }
    }
    channel->tcp.latest_wr_time = cdk_time_now();

    if (n > 0 && channel->handler->tcp.on_write) {
        cdk_net_post_event(channel->poller, _write_complete_cb, channel, true);
    }
}

static void
_udp_explicit_send(cdk_channel_t* channel, void* data, size_t size) {
    ssize_t n = 0;
    if (txlist_empty(&channel->txlist)) {
        if (channel->side == SIDE_CLIENT) {
            n = platform_socket_send(channel->fd, data, size);
        }
        if (channel->side == SIDE_SERVER) {
            n = platform_socket_sendto(
                channel->fd,
                data,
                size,
                &(channel->udp.peer.ss),
                channel->udp.peer.sslen);
        }
        if (n == PLATFORM_SO_ERROR_SOCKET_ERROR) {
            if ((platform_socket_lasterror() == PLATFORM_SO_ERROR_EAGAIN) ||
                (platform_socket_lasterror() ==
                 PLATFORM_SO_ERROR_EWOULDBLOCK)) {

                txlist_insert(&channel->txlist, data, size, true);
                if (!channel_is_writing(channel)) {
                    channel_enable_write(channel);
                }
                return;
            }
            channel_destroy(
                channel,
                CHANNEL_REASON_SYSCALL_FAIL,
                platform_socket_error2string(platform_socket_lasterror()));
            return;
        }
    }
    if (n > 0 && channel->handler->udp.on_write) {
        cdk_net_post_event(channel->poller, _write_complete_cb, channel, true);
    }
}

static void _tls_recv(cdk_channel_t* channel) {
    int err = 0;
    int n = tls_ssl_read(
        channel->tcp.tls_ssl,
        (char*)(channel->rxbuf.buf) + channel->rxbuf.off,
        MAX_TCP_RECVBUF_SIZE,
        &err);
    if (n <= 0) {
        if (n == 0) {
            return;
        }
        channel_destroy(
            channel, CHANNEL_REASON_TLS_FAIL, tls_error2string(err));
        return;
    }
    channel->tcp.latest_rd_time = cdk_time_now();
    channel->rxbuf.off += n;
    if (channel->rxbuf.off > MAX_TCP_RECVBUF_SIZE) {
        abort();
    }
    unpacker_unpack(channel);
}

// static void _dtls_recv(cdk_channel_t* channel) {
//     int err = 0;
//     int n = 0;
//     if (channel->type == SOCK_STREAM) {
//         n = tls_ssl_read(
//             channel->tcp.ssl,
//             (char*)(channel->rxbuf.buf) + channel->rxbuf.off,
//             MAX_TCP_RECVBUF_SIZE,
//             &err);
//     }
//     if (channel->type == SOCK_DGRAM) {
//         n = tls_ssl_read(
//             channel->udp.ssl->dtls_ssl,
//             channel->rxbuf.buf,
//             MAX_UDP_RECVBUF_SIZE,
//             &err);
//     }
//     if (n <= 0) {
//         if (n == 0) {
//             return;
//         }
//         channel_destroy(channel, CHANNEL_REASON_TLS_FAIL,
//         tls_error2string(err)); return;
//     }
//     if (channel->type == SOCK_STREAM) {
//         channel->tcp.latest_rd_time = cdk_time_now();
//         channel->rxbuf.off += n;
//         if (channel->rxbuf.off > MAX_TCP_RECVBUF_SIZE) {
//             abort();
//         }
//         unpacker_unpack(channel);
//     }
//     if (channel->type == SOCK_DGRAM) {
//         if (channel->handler->udp.on_read) {
//             channel->handler->udp.on_read(channel, channel->rxbuf.buf, n);
//         }
//     }
// }

static void _tcp_recv(cdk_channel_t* channel) {
    ssize_t n = platform_socket_recv(
        channel->fd,
        (char*)(channel->rxbuf.buf) + channel->rxbuf.off,
        MAX_TCP_RECVBUF_SIZE);
    if (n == PLATFORM_SO_ERROR_SOCKET_ERROR) {
        if ((platform_socket_lasterror() == PLATFORM_SO_ERROR_EAGAIN) ||
            (platform_socket_lasterror() == PLATFORM_SO_ERROR_EWOULDBLOCK)) {
            return;
        }
        channel_destroy(
            channel,
            CHANNEL_REASON_SYSCALL_FAIL,
            platform_socket_error2string(platform_socket_lasterror()));
        return;
    }
    if (n == 0) {
        channel_destroy(
            channel,
            CHANNEL_REASON_SYSCALL_FAIL,
            platform_socket_error2string(PLATFORM_SO_ERROR_ECONNRESET));
        return;
    }
    channel->tcp.latest_rd_time = cdk_time_now();
    channel->rxbuf.off += n;
    unpacker_unpack(channel);
}

static void _udp_recv(cdk_channel_t* channel) {
    ssize_t n;
    if (channel->side == SIDE_CLIENT) {
        n = platform_socket_recv(
            channel->fd, channel->rxbuf.buf, MAX_UDP_RECVBUF_SIZE);
    }
    if (channel->side == SIDE_SERVER) {
        channel->udp.peer.sslen = sizeof(struct sockaddr_storage);
        n = platform_socket_recvfrom(
            channel->fd,
            channel->rxbuf.buf,
            MAX_UDP_RECVBUF_SIZE,
            &channel->udp.peer.ss,
            &channel->udp.peer.sslen);
    }
    if (n == PLATFORM_SO_ERROR_SOCKET_ERROR) {
        if ((platform_socket_lasterror() == PLATFORM_SO_ERROR_EAGAIN) ||
            (platform_socket_lasterror() == PLATFORM_SO_ERROR_EWOULDBLOCK)) {
            return;
        }
        channel_destroy(
            channel,
            CHANNEL_REASON_SYSCALL_FAIL,
            platform_socket_error2string(platform_socket_lasterror()));
        return;
    }
    if (channel->handler->udp.on_read) {
        channel->handler->udp.on_read(channel, channel->rxbuf.buf, n);
    }
}

static inline void _channel_cleanup_cb(void* param) {
    cdk_channel_t* channel = param;

    if (channel->ch_destroy_timer) {
        cdk_timer_del(&channel->poller->timermgr, channel->ch_destroy_timer);
    }
    free(channel);
    channel = NULL;
}

static inline void _channel_destroy_cb(void* param) {
    cdk_channel_t* channel = param;
    cdk_net_post_event(channel->poller, _channel_cleanup_cb, channel, true);
}

static inline void _rd_timeout_cb(void* param) {
    cdk_channel_t* channel = param;
    if ((cdk_time_now() - channel->tcp.latest_rd_time) >
        channel->handler->tcp.rd_timeout) {
        channel_destroy(
            channel, CHANNEL_REASON_RD_TIMEOUT, CHANNEL_REASON_RD_TIMEOUT_STR);
    }
}

static inline void _wr_timeout_cb(void* param) {
    cdk_channel_t* channel = param;
    if ((cdk_time_now() - channel->tcp.latest_wr_time) >
        channel->handler->tcp.wr_timeout) {
        channel_destroy(
            channel, CHANNEL_REASON_WR_TIMEOUT, CHANNEL_REASON_WR_TIMEOUT_STR);
    }
}

static inline void _heartbeat_cb(void* param) {
    cdk_channel_t* channel = param;
    if (channel->handler->tcp.on_heartbeat) {
        channel->handler->tcp.on_heartbeat(channel);
    }
}

static void _tcp_timers_create(cdk_channel_t* channel) {
    if (channel->handler->tcp.hb_interval) {
        channel->tcp.hb_timer = cdk_timer_add(
            &channel->poller->timermgr,
            _heartbeat_cb,
            channel,
            channel->handler->tcp.hb_interval,
            true);
    }
    if (channel->handler->tcp.rd_timeout) {
        channel->tcp.rd_timer = cdk_timer_add(
            &channel->poller->timermgr,
            _rd_timeout_cb,
            channel,
            channel->handler->tcp.rd_timeout,
            false);
    }
    if (channel->handler->tcp.wr_timeout) {
        channel->tcp.wr_timer = cdk_timer_add(
            &channel->poller->timermgr,
            _wr_timeout_cb,
            channel,
            channel->handler->tcp.wr_timeout,
            false);
    }
}

void channel_connected(cdk_channel_t* channel) {
    if (channel->type == SOCK_STREAM) {
        channel->tcp.connecting = false;

        if (channel->handler->tcp.on_connect) {
            channel->handler->tcp.on_connect(channel);
        }
        if (!channel_is_reading(channel)) {
            channel_enable_read(channel);
        }
        _tcp_timers_create(channel);
    }
    if (channel->type == SOCK_DGRAM) {
        if (channel->handler->udp.on_connect) {
            channel->handler->udp.on_connect(channel);
        }
        if (!channel_is_reading(channel)) {
            channel_enable_read(channel);
        }
    }
}

void channel_recv(cdk_channel_t* channel) {
    if (channel->type == SOCK_STREAM) {
        if (channel->tcp.tls_ssl) {
            _tls_recv(channel);
        } else {
            _tcp_recv(channel);
        }
    }
    if (channel->type == SOCK_DGRAM) {
        _udp_recv(channel);
    }
}

void channel_accepted(cdk_channel_t* channel) {
    if (channel->handler->tcp.on_accept) {
        channel->handler->tcp.on_accept(channel);
    }
    if (!channel_is_reading(channel)) {
        channel_enable_read(channel);
    }
    _tcp_timers_create(channel);
}

void channel_tls_cli_handshake(void* param) {
    int            err = 0;
    cdk_channel_t* channel = param;

    int n = tls_connect(channel->tcp.tls_ssl, channel->fd, &err);
    if (n <= 0) {
        if (n == 0) {
            cdk_net_post_event(
                channel->poller, channel_tls_cli_handshake, channel, true);
            return;
        }
        channel_destroy(
            channel, CHANNEL_REASON_TLS_FAIL, tls_error2string(err));
        return;
    }
    channel_connected(channel);
}

void channel_tls_srv_handshake(void* param) {
    int            err = 0;
    cdk_channel_t* channel = param;
    int n = tls_accept(channel->tcp.tls_ssl, channel->fd, true, &err);
    if (n <= 0) {
        if (n == 0) {
            cdk_net_post_event(
                channel->poller, channel_tls_srv_handshake, channel, true);
            return;
        }
        channel_destroy(
            channel, CHANNEL_REASON_TLS_FAIL, tls_error2string(err));
        return;
    }
    channel_accepted(channel);
}

cdk_channel_t* channel_create(
    cdk_poller_t*      poller,
    cdk_sock_t         sock,
    cdk_channel_mode_t mode,
    cdk_side_t         side,
    cdk_handler_t*     handler,
    cdk_tls_ctx_t*     tlsctx) {
    cdk_channel_t* channel = malloc(sizeof(cdk_channel_t));

    if (channel) {
        memset(channel, 0, sizeof(cdk_channel_t));
        channel->poller = poller;
        channel->fd = sock;
        channel->handler = handler;
        channel->type = platform_socket_getsocktype(sock);
        atomic_init(&channel->closing, false);
        txlist_create(&channel->txlist);
        channel->mode = mode;
        channel->side = side;

        if (channel->type == SOCK_STREAM) {
            channel->rxbuf.len = MAX_TCP_RECVBUF_SIZE;
        }
        if (channel->type == SOCK_DGRAM) {
            channel->rxbuf.len = MAX_UDP_RECVBUF_SIZE;
        }
        channel->rxbuf.off = 0;
        channel->rxbuf.buf = malloc(channel->rxbuf.len);
        if (channel->rxbuf.buf) {
            memset(channel->rxbuf.buf, 0, channel->rxbuf.len);
        }
        if (channel->type == SOCK_STREAM) {
            if (tlsctx) {
                if (channel->mode == CHANNEL_MODE_ACCEPT ||
                    channel->mode == CHANNEL_MODE_CONNECT) {
                    channel->tcp.tls_ctx = tlsctx;
                }
                if (channel->mode == CHANNEL_MODE_CONNECT ||
                    channel->mode == CHANNEL_MODE_NORMAL) {
                    channel->tcp.tls_ssl = tls_ssl_create(tlsctx);
                }
            }
        }
        cdk_list_insert_tail(&poller->chlist, &channel->node);
        return channel;
    }
    return NULL;
}

void channel_destroy(
    cdk_channel_t* channel, cdk_channel_reason_t code, const char* reason) {
    atomic_store(&channel->closing, true);
    channel_disable_all(channel);
    platform_socket_close(channel->fd);
    cdk_list_remove(&channel->node);

    txlist_destroy(&channel->txlist);
    free(channel->rxbuf.buf);
    channel->rxbuf.buf = NULL;
    channel->rxbuf.len = 0;
    channel->rxbuf.off = 0;

    if (channel->type == SOCK_STREAM) {
        if (channel->tcp.accepting || channel->tcp.connecting) {
            tls_ctx_destroy(channel->tcp.tls_ctx);
        } else {
            tls_ssl_destroy(channel->tcp.tls_ssl);
        }
        if (channel->tcp.hb_timer) {
            cdk_timer_del(&channel->poller->timermgr, channel->tcp.hb_timer);
        }
        if (channel->tcp.rd_timer) {
            cdk_timer_del(&channel->poller->timermgr, channel->tcp.rd_timer);
        }
        if (channel->tcp.wr_timer) {
            cdk_timer_del(&channel->poller->timermgr, channel->tcp.wr_timer);
        }
        if (channel->handler->tcp.on_close) {
            channel->handler->tcp.on_close(channel, code, reason);
        }
    }
    if (channel->type == SOCK_DGRAM) {
        if (channel->handler->udp.on_close) {
            channel->handler->udp.on_close(channel, code, reason);
        }
    }
    if (channel->poller->active) {
        channel->ch_destroy_timer = cdk_timer_add(
            &channel->poller->timermgr,
            _channel_destroy_cb,
            channel,
            CHANNEL_DELAYED_DESTROY_TIME,
            false);
    } else {
        if (channel) {
            free(channel);
            channel = NULL;
        }
    }
}

void channel_send(cdk_channel_t* channel) {
    if (channel->type == SOCK_STREAM) {
        if (channel->tcp.tls_ssl) {
            _tls_send(channel);
        } else {
            _tcp_send(channel);
        }
    }
    if (channel->type == SOCK_DGRAM) {
        _udp_send(channel);
    }
}

void channel_accept(cdk_channel_t* channel) {
    cdk_sock_t cli = platform_socket_accept(channel->fd, true);
    if (cli == PLATFORM_SO_ERROR_INVALID_SOCKET) {
        if ((platform_socket_lasterror() == PLATFORM_SO_ERROR_EAGAIN) ||
            (platform_socket_lasterror() == PLATFORM_SO_ERROR_EWOULDBLOCK)) {
            return;
        }
        channel_destroy(
            channel,
            CHANNEL_REASON_SYSCALL_FAIL,
            platform_socket_error2string(platform_socket_lasterror()));
        return;
    }
    cdk_channel_t* nchannel = channel_create(
        global_net_engine.poller_roundrobin(),
        cli,
        CHANNEL_MODE_NORMAL,
        SIDE_SERVER,
        channel->handler,
        channel->tcp.tls_ctx);
    if (nchannel) {
        if (nchannel->tcp.tls_ssl) {
            channel_tls_srv_handshake(nchannel);
        } else {
            channel_accepted(nchannel);
        }
    }
}

void channel_connect(cdk_channel_t* channel) {
    if (atomic_load(&channel->closing)) {
        return;
    }
    int       err = 0;
    socklen_t len = sizeof(int);

    channel_disable_all(channel);
    if (channel->handler->tcp.conn_timeout) {
        cdk_timer_del(&channel->poller->timermgr, channel->tcp.conn_timer);
    }
    getsockopt(channel->fd, SOL_SOCKET, SO_ERROR, (char*)&err, &len);
    if (err) {
        channel_destroy(
            channel,
            CHANNEL_REASON_SYSCALL_FAIL,
            platform_socket_error2string(err));
    } else {
        if (channel->tcp.tls_ssl) {
            channel_tls_cli_handshake(channel);
        } else {
            channel_connected(channel);
        }
    }
}

void channel_enable_write(cdk_channel_t* channel) {
    if (channel->events) {
        platform_event_mod(
            channel->poller->pfd,
            channel->fd,
            channel->events | EVENT_WR,
            channel);
    } else {
        platform_event_add(
            channel->poller->pfd,
            channel->fd,
            channel->events | EVENT_WR,
            channel);
    }
    channel->events |= EVENT_WR;
}

void channel_enable_read(cdk_channel_t* channel) {
    if (channel->events) {
        platform_event_mod(
            channel->poller->pfd,
            channel->fd,
            channel->events | EVENT_RD,
            channel);
    } else {
        platform_event_add(
            channel->poller->pfd,
            channel->fd,
            channel->events | EVENT_RD,
            channel);
    }
    channel->events |= EVENT_RD;
}

void channel_disable_write(cdk_channel_t* channel) {
    if (channel->events) {
        platform_event_mod(
            channel->poller->pfd,
            channel->fd,
            channel->events & ~EVENT_WR,
            channel);
    } else {
        platform_event_del(channel->poller->pfd, channel->fd);
    }
    channel->events &= ~EVENT_WR;
}

void channel_disable_read(cdk_channel_t* channel) {
    if (channel->events) {
        platform_event_mod(
            channel->poller->pfd,
            channel->fd,
            channel->events & ~EVENT_RD,
            channel);
    } else {
        platform_event_del(channel->poller->pfd, channel->fd);
    }
    channel->events &= ~EVENT_RD;
}

void channel_disable_all(cdk_channel_t* channel) {
    platform_event_del(channel->poller->pfd, channel->fd);
    channel->events = 0;
}

bool channel_is_writing(cdk_channel_t* channel) {
    return channel->events & EVENT_WR;
}

bool channel_is_reading(cdk_channel_t* channel) {
    return channel->events & EVENT_RD;
}

void channel_explicit_send(cdk_channel_t* channel, void* data, size_t size) {
    if (channel->type == SOCK_STREAM) {
        if (channel->tcp.tls_ssl) {
            _tls_explicit_send(channel, data, size);
        } else {
            _tcp_explicit_send(channel, data, size);
        }
    }
    if (channel->type == SOCK_DGRAM) {
        _udp_explicit_send(channel, data, size);
    }
}
