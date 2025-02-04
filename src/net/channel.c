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
#include "cdk/cdk-logger.h"

#define CHANNEL_DELAYED_DESTROY_TIME 10000

extern cdk_net_engine_t global_net_engine;

static inline void _write_complete_cb(void* param) {
    cdk_channel_t* channel = param;
    channel->handler->on_write(channel);
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
        cdk_channel_error_t error = {
            .code = CHANNEL_ERROR_SYSCALL_FAIL,
            .codestr =
                platform_socket_error2string(platform_socket_lasterror())
        };
        channel_error_update(channel, error);
        channel_destroy(channel);
        return;
    }
    if (n < e->len) {
        txlist_insert(&channel->txlist, e->buf + n, (e->len - n), false);
    }
    channel->latest_wr_time = cdk_time_now();
    if (n > 0 && channel->handler->on_write) {
        channel->handler->on_write(channel);
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
    } else {
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
        cdk_channel_error_t error = {
            .code = CHANNEL_ERROR_SYSCALL_FAIL,
            .codestr =
                platform_socket_error2string(platform_socket_lasterror())};
        channel_error_update(channel, error);
        channel_destroy(channel);
        return;
    }
    channel->latest_wr_time = cdk_time_now();

    if (n > 0 && channel->handler->on_write) {
        channel->handler->on_write(channel);
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
        cdk_channel_error_t error = {
            .code = CHANNEL_ERROR_TLS_FAIL, 
            .codestr = tls_error2string(err)
        };
        channel_error_update(channel, error);
        channel_destroy(channel);
        return;
    }
    if (n < e->len) {
        txlist_insert(&channel->txlist, e->buf + n, (e->len - n), false);
    }
    channel->latest_wr_time = cdk_time_now();
    if (n > 0 && channel->handler->on_write) {
        channel->handler->on_write(channel);
    }
    txlist_remove(e);
}

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
            cdk_channel_error_t error = {
                .code = CHANNEL_ERROR_TLS_FAIL,
                .codestr = tls_error2string(err)};
            channel_error_update(channel, error);

            channel_destroy(channel);
            return;
        }
    }
    if (n < size) {
        txlist_insert(&channel->txlist, (char*)data + n, (size - n), true);
        if (!channel_is_writing(channel)) {
            channel_enable_write(channel);
        }
    }
    channel->latest_wr_time = cdk_time_now();
    if (n > 0 && channel->handler->on_write) {
        cdk_net_post_event(channel->poller, _write_complete_cb, channel, true);
    }
}

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
            cdk_channel_error_t error = {
                .code = CHANNEL_ERROR_SYSCALL_FAIL,
                .codestr =
                    platform_socket_error2string(platform_socket_lasterror())
            };
            channel_error_update(channel, error);
            channel_destroy(channel);
            return;
        }
    }
    if (n < size) {
        txlist_insert(&channel->txlist, (char*)data + n, (size - n), true);
        if (!channel_is_writing(channel)) {
            channel_enable_write(channel);
        }
    }
    channel->latest_wr_time = cdk_time_now();

    if (n > 0 && channel->handler->on_write) {
        cdk_net_post_event(channel->poller, _write_complete_cb, channel, true);
    }
}

static void
_udp_explicit_send(cdk_channel_t* channel, void* data, size_t size) {
    ssize_t n = 0;
    if (txlist_empty(&channel->txlist)) {
        if (channel->side == SIDE_CLIENT) {
            n = platform_socket_send(channel->fd, data, size);
        } else {
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
            cdk_channel_error_t error = {
                .code = CHANNEL_ERROR_SYSCALL_FAIL,
                .codestr =
                    platform_socket_error2string(platform_socket_lasterror())};
            channel_error_update(channel, error);
            channel_destroy(channel);
            return;
        }
    }
    if (n > 0 && channel->handler->on_write) {
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
        cdk_channel_error_t error = {
            .code = CHANNEL_ERROR_TLS_FAIL, .codestr = tls_error2string(err)};
        channel_error_update(channel, error);
        channel_destroy(channel);
        return;
    }
    channel->latest_rd_time = cdk_time_now();
    channel->rxbuf.off += n;
    if (channel->rxbuf.off > MAX_TCP_RECVBUF_SIZE) {
        abort();
    }
    unpacker_unpack(channel);
}

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
        cdk_channel_error_t error = {
            .code = CHANNEL_ERROR_SYSCALL_FAIL,
            .codestr =
                platform_socket_error2string(platform_socket_lasterror())};
        channel_error_update(channel, error);

        channel_destroy(channel);
        return;
    }
    if (n == 0) {
        cdk_channel_error_t error = {
            .code = CHANNEL_ERROR_SYSCALL_FAIL,
            .codestr =
                platform_socket_error2string(PLATFORM_SO_ERROR_ECONNRESET)};
        channel_error_update(channel, error);
        channel_destroy(channel);
        return;
    }
    channel->latest_rd_time = cdk_time_now();
    channel->rxbuf.off += n;
    unpacker_unpack(channel);
}

static void _udp_recv(cdk_channel_t* channel) {
    ssize_t n;
    if (channel->side == SIDE_CLIENT) {
        n = platform_socket_recv(
            channel->fd, channel->rxbuf.buf, MAX_UDP_RECVBUF_SIZE);
    } else {
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
        cdk_channel_error_t error = {
            .code = CHANNEL_ERROR_SYSCALL_FAIL,
            .codestr =
                platform_socket_error2string(platform_socket_lasterror())};
        channel_error_update(channel, error);
        channel_destroy(channel);
        return;
    }
    channel->latest_rd_time = cdk_time_now();
    if (channel->handler->on_read) {
        channel->handler->on_read(channel, channel->rxbuf.buf, n);
    }
}

static inline void _rd_timeout_cb(void* param) {
    cdk_channel_t* channel = param;

    uint64_t elapsed_time = cdk_time_now() - channel->latest_rd_time;
    if (elapsed_time > channel->handler->rd_timeout) {
        cdk_channel_error_t error = {
            .code = CHANNEL_ERROR_RD_TIMEOUT,
            .codestr = CHANNEL_ERROR_RD_TIMEOUT_STR
        };
        channel_error_update(channel, error);
        channel_destroy(channel);
    } else {
        channel->rd_timer->expire = (channel->handler->rd_timeout - elapsed_time);
    }
}

static inline void _wr_timeout_cb(void* param) {
    cdk_channel_t* channel = param;

    uint64_t elapsed_time = cdk_time_now() - channel->latest_wr_time;
    if (elapsed_time > channel->handler->wr_timeout) {
        cdk_channel_error_t error = {
            .code = CHANNEL_ERROR_WR_TIMEOUT,
            .codestr = CHANNEL_ERROR_WR_TIMEOUT_STR};
        channel_error_update(channel, error);
        channel_destroy(channel);
    } else {
        channel->wr_timer->expire = (channel->handler->wr_timeout - elapsed_time);
    }
}

static inline void _heartbeat_cb(void* param) {
    cdk_channel_t* channel = param;
    if (channel->handler->on_heartbeat) {
        channel->handler->on_heartbeat(channel);
    }
}

void channel_timers_destroy(cdk_channel_t* channel) {
    if (channel->hb_timer) {
        cdk_timer_del(channel->poller->timermgr, channel->hb_timer);
    }
    if (channel->rd_timer) {
        cdk_timer_del(channel->poller->timermgr, channel->rd_timer);
    }
    if (channel->wr_timer) {
        cdk_timer_del(channel->poller->timermgr, channel->wr_timer);
    }
}

static inline void _channel_destroy_cb(void* param) {
    cdk_channel_t* channel = param;

    txlist_destroy(&channel->txlist);

    free(channel->rxbuf.buf);
    channel->rxbuf.buf = NULL;
    channel->rxbuf.len = 0;
    channel->rxbuf.off = 0;

    if (channel->type == SOCK_STREAM) {
        if (channel->mode == CHANNEL_MODE_ACCEPT ||
            channel->mode == CHANNEL_MODE_CONNECT) {
            tls_ctx_destroy(channel->tcp.tls_ctx);
        }
        if (channel->mode == CHANNEL_MODE_CONNECT ||
            channel->mode == CHANNEL_MODE_NORMAL) {
            tls_ssl_destroy(channel->tcp.tls_ssl);
        }
    }
    channel_timers_destroy(channel);
    if (channel) {
        free(channel);
        channel = NULL;
    }
}

void channel_timers_create(cdk_channel_t* channel) {
    if (channel->handler->hb_interval) {
        channel->hb_timer = cdk_timer_add(
            channel->poller->timermgr,
            _heartbeat_cb,
            channel,
            channel->handler->hb_interval,
            true);
    }
    if (channel->handler->rd_timeout) {
        channel->rd_timer = cdk_timer_add(
            channel->poller->timermgr,
            _rd_timeout_cb,
            channel,
            channel->handler->rd_timeout,
            true);
    }
    if (channel->handler->wr_timeout) {
        channel->wr_timer = cdk_timer_add(
            channel->poller->timermgr,
            _wr_timeout_cb,
            channel,
            channel->handler->wr_timeout,
            true);
    }
}

void channel_connected(cdk_channel_t* channel) {
    if (channel->handler->on_connect) {
        channel->handler->on_connect(channel);
    }
    if (!channel_is_reading(channel)) {
        channel_enable_read(channel);
    }
    channel_timers_create(channel);
}

void channel_recv(cdk_channel_t* channel) {
    if (atomic_load(&channel->closing)) {
        return;
    }
    if (channel->type == SOCK_STREAM) {
        if (channel->tcp.tls_ssl) {
            _tls_recv(channel);
        } else {
            _tcp_recv(channel);
        }
    } else {
        _udp_recv(channel);
    }
}

void channel_accepted(cdk_channel_t* channel) {
    if (channel->handler->on_accept) {
        channel->handler->on_accept(channel);
    }
    if (!channel_is_reading(channel)) {
        channel_enable_read(channel);
    }
    channel_timers_create(channel);
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
        cdk_channel_error_t error = {
            .code = CHANNEL_ERROR_TLS_FAIL, 
            .codestr = tls_error2string(err)
        };
        channel_error_update(channel, error);

        channel_destroy(channel);
        return;
    }
    channel_connected(channel);
}

void channel_error_update(cdk_channel_t* channel, cdk_channel_error_t error) {
    channel->error.code = error.code;
    channel->error.codestr = (char*)error.codestr;
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
        cdk_channel_error_t error = {
            .code = CHANNEL_ERROR_TLS_FAIL, 
            .codestr = tls_error2string(err)
        };
        channel_error_update(channel, error);
        channel_destroy(channel);
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
        } else {
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

void channel_destroy(cdk_channel_t* channel) {
    if (atomic_load(&channel->closing)) {
        return;
    }
    atomic_store(&channel->closing, true);
    channel_disable_all(channel);
    platform_socket_close(channel->fd);
    cdk_list_remove(&channel->node);

    if (channel->handler->on_close) {
        channel->handler->on_close(channel, channel->error);
    }
    cdk_timer_add(
        channel->poller->timermgr,
        _channel_destroy_cb,
        channel,
        CHANNEL_DELAYED_DESTROY_TIME,
        false);
}

void channel_send(cdk_channel_t* channel) {
    if (atomic_load(&channel->closing)) {
        return;
    }
    if (channel->type == SOCK_STREAM) {
        if (channel->tcp.tls_ssl) {
            _tls_send(channel);
        } else {
            _tcp_send(channel);
        }
    } else {
        _udp_send(channel);
    }
}

void channel_accepting(cdk_channel_t* channel) {
    if (atomic_load(&channel->closing)) {
        return;
    }
    cdk_sock_t cli = platform_socket_accept(channel->fd, true);
    if (cli == PLATFORM_SO_ERROR_INVALID_SOCKET) {
        if ((platform_socket_lasterror() == PLATFORM_SO_ERROR_EAGAIN) ||
            (platform_socket_lasterror() == PLATFORM_SO_ERROR_EWOULDBLOCK)) {
            return;
        }
        cdk_channel_error_t error = {
            .code = CHANNEL_ERROR_SYSCALL_FAIL,
            .codestr =
                platform_socket_error2string(platform_socket_lasterror())
        };
        channel_error_update(channel, error);
        channel_destroy(channel);
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

void channel_connecting(cdk_channel_t* channel) {
    if (atomic_load(&channel->closing)) {
        if (channel->handler->conn_timeout) {
            cdk_timer_del(channel->poller->timermgr, channel->tcp.conn_timer);
        }
        return;
    }
    int       err = 0;
    socklen_t len = sizeof(int);

    channel_disable_all(channel);
    if (channel->handler->conn_timeout) {
        cdk_timer_del(channel->poller->timermgr, channel->tcp.conn_timer);
    }
    getsockopt(channel->fd, SOL_SOCKET, SO_ERROR, (char*)&err, &len);
    if (err) {
        cdk_channel_error_t error = {
            .code = CHANNEL_ERROR_SYSCALL_FAIL,
            .codestr =
                platform_socket_error2string(platform_socket_lasterror())};
        channel_error_update(channel, error);
        channel_destroy(channel);
    } else {
        channel->tcp.connecting = false;
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
    } else {
        _udp_explicit_send(channel, data, size);
    }
}
