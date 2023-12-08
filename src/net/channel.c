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
#include "channel.h"
#include "unpack.h"
#include "tls.h"
#include "txlist.h"
#include "cdk/cdk-timer.h"
#include "cdk/cdk-time.h"
#include "cdk/net/cdk-net.h"
#include "cdk/container/cdk-rbtree.h"

extern cdk_timer_t timer;
extern cdk_poller_t* _poller_roundrobin(void);

static void _allocate_io_buffers(cdk_offset_buf_t* in, cdk_list_t* out) {
    in->len = DEFAULT_IOBUF_SIZE;
    in->off = 0;
    in->buf = malloc(DEFAULT_IOBUF_SIZE);
    if (in->buf) {
        memset(in->buf, 0, DEFAULT_IOBUF_SIZE);
    }
    txlist_create(out);
}

static void _deallocate_io_buffers(cdk_offset_buf_t* in, cdk_list_t* out) {
    free(in->buf);
    in->buf = NULL;
    in->len = 0;
    in->off = 0;

    txlist_destroy(out);
}

static void _write_complete_callback(void* param) {
    cdk_channel_t* channel = param;
    channel->handler->on_write(channel);
}

static void _channel_tcp_unencrypted_send(cdk_channel_t* channel) {
    if (txlist_empty(&channel->txlist)) {
        if (channel_is_writing(channel)) {
            channel_disable_write(channel);
        }
        return;
    }
    txlist_node_t* e = cdk_list_data(cdk_list_head(&(channel->txlist)), txlist_node_t, n);

    ssize_t n = platform_socket_send(channel->fd, e->buf, (int)e->len);
    if (n == -1) {
        if ((platform_socket_lasterror() == PLATFORM_SO_ERROR_EAGAIN)
            || (platform_socket_lasterror() == PLATFORM_SO_ERROR_EWOULDBLOCK)) {
            return;
        }
        channel->handler->on_close(channel, platform_socket_error2string(platform_socket_lasterror()));
        return;
    }
    if (n < e->len) {
        txlist_insert(&channel->txlist, e->buf + n, (e->len - n), false);
    }
    if (n > 0) {
        cdk_net_postevent(channel->poller, _write_complete_callback, channel, true);
    }
    txlist_remove(e);
}

static void _channel_tcp_encrypted_send(cdk_channel_t* channel) {
    int err = 0;
    if (txlist_empty(&channel->txlist)) {
        if (channel_is_writing(channel)) {
            channel_disable_write(channel);
        }
        return;
    }
    txlist_node_t* e = cdk_list_data(cdk_list_head(&(channel->txlist)), txlist_node_t, n);
    int n = tls_write(channel->tls, e->buf, (int)e->len, &err);
    if (n <= 0) {
        if (n == 0) {
            return;
        }
        channel->handler->on_close(channel, tls_error2string(err));
        return;
    }
    if (n < e->len) {
        txlist_insert(&channel->txlist, e->buf + n, (e->len - n), false);
    }
    cdk_net_postevent(channel->poller, _write_complete_callback, channel, true);
    txlist_remove(e);
}

static void _channel_udp_encrypted_send(cdk_channel_t* channel) {

}

static void _channel_udp_unencrypted_send(cdk_channel_t* channel) {
    if (txlist_empty(&channel->txlist)) {
        if (channel_is_writing(channel)) {
            channel_disable_write(channel);
        }
        return;
    }
    txlist_node_t* e = cdk_list_data(cdk_list_head(&(channel->txlist)), txlist_node_t, n);

    ssize_t n = platform_socket_sendto(channel->fd, e->buf, (int)e->len, &(channel->udp.peer.ss), channel->udp.peer.sslen);
    if (n == -1) {
        if ((platform_socket_lasterror() == PLATFORM_SO_ERROR_EAGAIN)
            || (platform_socket_lasterror() == PLATFORM_SO_ERROR_EWOULDBLOCK)) {
            return;
        }
        channel->handler->on_close(channel, platform_socket_error2string(platform_socket_lasterror()));
        return;
    }
    if (n < e->len) {
        txlist_insert(&channel->txlist, e->buf + n, (e->len - n), false);
    }
    if (n > 0) {
        cdk_net_postevent(channel->poller, _write_complete_callback, channel, true);
    }
    txlist_remove(e);
}

static void _channel_udp_encrypted_recv(cdk_channel_t* channel) {

}

static void _channel_udp_unencrypted_recv(cdk_channel_t* channel) {
    channel->udp.peer.sslen = sizeof(struct sockaddr_storage);
    ssize_t n = platform_socket_recvfrom(channel->fd, channel->rxbuf.buf, DEFAULT_IOBUF_SIZE, &channel->udp.peer.ss, &channel->udp.peer.sslen);
    if (n == -1) {
        /**
         * to be compatible with the semantics of unix, windows's WSAECONNRESET is filtered out.
         */
        if (((platform_socket_lasterror() == PLATFORM_SO_ERROR_EAGAIN)
            || (platform_socket_lasterror() == PLATFORM_SO_ERROR_EWOULDBLOCK))
            || (platform_socket_lasterror() == PLATFORM_SO_ERROR_ECONNRESET)) {
            return;
        }
        int ret = platform_socket_lasterror();
        channel->handler->on_close(channel, platform_socket_error2string(platform_socket_lasterror()));
        return;
    }
    channel->handler->on_read(channel, channel->rxbuf.buf, n);
}

static void _channel_tcp_encrypted_recv(cdk_channel_t* channel) {
    int err = 0;
    int n = tls_read(channel->tls, (char*)(channel->rxbuf.buf) + channel->rxbuf.off, DEFAULT_IOBUF_SIZE / 2, &err);
    if (n <= 0) {
        if (n == 0) {
            return;
        }
        channel->handler->on_close(channel, tls_error2string(err));
        return;
    }
    channel->rxbuf.off += n;
    unpack(channel);
}

static void _channel_tcp_unencrypted_recv(cdk_channel_t* channel) {
    ssize_t n = platform_socket_recv(channel->fd, (char*)(channel->rxbuf.buf) + channel->rxbuf.off, DEFAULT_IOBUF_SIZE / 2);
    if (n == -1) {
        if ((platform_socket_lasterror() == PLATFORM_SO_ERROR_EAGAIN)
            || (platform_socket_lasterror() == PLATFORM_SO_ERROR_EWOULDBLOCK)) {
            return;
        }
        channel->handler->on_close(channel, platform_socket_error2string(platform_socket_lasterror()));
        return;
    }
    /**
     * windows provides the WSAECONNRESET to detect a peer disconnection,
     * so it is not necessary to rely on receiving 0 from recv to determine that the connection has been closed.
     * but it is necessary to unix.
     */
    if (n == 0) {
        channel->handler->on_close(channel, platform_socket_error2string(PLATFORM_SO_ERROR_ECONNRESET));
        return;
    }
    channel->rxbuf.off += n;
    unpack(channel);
}

static void _channel_tcp_send(cdk_channel_t* channel) {
    if (channel->tls) {
        _channel_tcp_encrypted_send(channel);
    }
    else {
        _channel_tcp_unencrypted_send(channel);
    }
}

static void _channel_udp_send(cdk_channel_t* channel) {
    if (channel->tls) {
        _channel_udp_encrypted_send(channel);
    }
    else {
        _channel_udp_unencrypted_send(channel);
    }
}

static void _channel_tcp_recv(cdk_channel_t* channel) {
    if (channel->tls) {
        _channel_tcp_encrypted_recv(channel);
    }
    else {
        _channel_tcp_unencrypted_recv(channel);
    }
}

static void _channel_udp_recv(cdk_channel_t* channel) {
    if (channel->tls) {
        _channel_udp_encrypted_recv(channel);
    }
    else {
        _channel_udp_unencrypted_recv(channel);
    }
}

void channel_tls_cli_handshake(void* param) {
    int err = 0;
    cdk_channel_t* channel = param;
    int n = tls_connect(channel->tls, channel->fd, &err);
    if (n <= 0) {
        if (n == 0) {
            cdk_net_postevent(channel->poller, channel_tls_cli_handshake, channel, true);
            return;
        }
        channel->handler->on_close(channel, tls_error2string(err));
        return;
    }
    if (channel_is_connecting(channel)) {
        channel_disable_connect(channel);
    }
    channel->handler->on_connect(channel);
    if (!channel_is_reading(channel)) {
        channel_enable_read(channel);
    }
}

void channel_tls_srv_handshake(void* param) {
    int err = 0;
    cdk_channel_t* channel = param;
    int n = tls_accept(channel->tls, channel->fd, &err);
    if (n <= 0) {
        if (n == 0) {
            cdk_net_postevent(channel->poller, channel_tls_srv_handshake, channel, true);
            return;
        }
        channel->handler->on_close(channel, tls_error2string(err));
        return;
    }
    channel->handler->on_accept(channel);
    if (!channel_is_reading(channel)) {
        channel_enable_read(channel);
    }
}

cdk_channel_t* channel_create(cdk_poller_t* poller, cdk_sock_t sock, cdk_handler_t* handler)
{
    cdk_channel_t* channel = malloc(sizeof(cdk_channel_t));

    if (channel) {
        memset(channel, 0, sizeof(cdk_channel_t));
        channel->poller = poller;
        channel->fd = sock;
        channel->handler = handler;
        channel->type = platform_socket_socktype(sock);
        atomic_init(&channel->closing, false);
        
        _allocate_io_buffers(&channel->rxbuf, &channel->txlist);
        if (channel->handler->tlsconf) {
            if (channel->type == SOCK_STREAM) {
                channel->tls = tls_create(&channel->handler->tlsconf, ENABLE_TLS);
            }
            if (channel->type == SOCK_DGRAM) {
                channel->tls = tls_create(&channel->handler->tlsconf, ENABLE_DTLS);
            }
        }
        return channel;
    }
    return NULL;
}

void channel_destroy(cdk_channel_t* channel) {
    atomic_store(&channel->closing, true);
    
    platform_event_del(channel->poller->pfd, channel->fd, channel->events, channel);
    channel->events = 0;
    
    platform_socket_close(channel->fd);
    _deallocate_io_buffers(&channel->rxbuf, &channel->txlist);

    if (channel->type == SOCK_STREAM) {
        tls_destroy(channel->tls);
    }
    if (channel->type == SOCK_DGRAM) {
        
    }
    free(channel);
    channel = NULL;
}

void channel_recv(cdk_channel_t* channel) {
    if (channel->type == SOCK_STREAM) {
        _channel_tcp_recv(channel);
    }
    if (channel->type == SOCK_DGRAM) {
        _channel_udp_recv(channel);
    }
}

void channel_send(cdk_channel_t* channel) {
    if (channel->type == SOCK_STREAM) {
        _channel_tcp_send(channel);
    }
    if (channel->type == SOCK_DGRAM) {
        _channel_udp_send(channel);
    }
}

void channel_accept(cdk_channel_t* channel) {
    cdk_sock_t cli = platform_socket_accept(channel->fd);
    if (cli == -1) {
        if ((platform_socket_lasterror() == PLATFORM_SO_ERROR_EAGAIN)
            || (platform_socket_lasterror() == PLATFORM_SO_ERROR_EWOULDBLOCK)) {
            return;
        }
        channel->handler->on_close(channel, platform_socket_error2string(platform_socket_lasterror()));
        return;
    }
    cdk_channel_t* newchannel = channel_create(_poller_roundrobin(), cli, channel->handler);
    if (newchannel) {
        if (newchannel->tls) {
            _do_srv_handshake(newchannel);
        } else {
            newchannel->handler->on_accept(newchannel);
            if (!channel_is_reading(newchannel)) {
                channel_enable_read(newchannel);
            }
        }
    }
}

void channel_connect(cdk_channel_t* channel) {
    int err;
    socklen_t len;
    len = sizeof(int);

    cdk_timer_del(&timer, channel->tcp.ctimer);

    getsockopt(channel->fd, SOL_SOCKET, SO_ERROR, (char*)&err, &len);
    if (err) {
        channel->handler->on_close(channel, platform_socket_error2string(err));
    } else {
        if (channel->tls) {
            channel_tls_cli_handshake(channel);
        } else {
            if (channel_is_connecting(channel)) {
                channel_disable_connect(channel);
            }
            channel->handler->on_connect(channel);
            if (!channel_is_reading(channel)) {
                channel_enable_read(channel);
            }
        }
    }
}

void channel_enable_accept(cdk_channel_t* channel) {
    platform_event_add(channel->poller->pfd, channel->fd, EVENT_TYPE_A, channel);
    channel->events |= EVENT_TYPE_A;
}

void channel_enable_connect(cdk_channel_t* channel) {
    platform_event_add(channel->poller->pfd, channel->fd, EVENT_TYPE_C, channel);
    channel->events |= EVENT_TYPE_C;
}

void channel_enable_write(cdk_channel_t* channel) {
    platform_event_add(channel->poller->pfd, channel->fd, EVENT_TYPE_W, channel);
    channel->events |= EVENT_TYPE_W;
}

void channel_enable_read(cdk_channel_t* channel) {
    platform_event_add(channel->poller->pfd, channel->fd, EVENT_TYPE_R, channel);
    channel->events |= EVENT_TYPE_R;
}

void channel_disable_accept(cdk_channel_t* channel) {
    platform_event_del(channel->poller->pfd, channel->fd, EVENT_TYPE_A, channel);
    channel->events &= ~EVENT_TYPE_A;
}

void channel_disable_connect(cdk_channel_t* channel) {
    platform_event_del(channel->poller->pfd, channel->fd, EVENT_TYPE_C, channel);
    channel->events &= ~EVENT_TYPE_C;
}

void channel_disable_write(cdk_channel_t* channel) {
    platform_event_del(channel->poller->pfd, channel->fd, EVENT_TYPE_W, channel);
    channel->events &= ~EVENT_TYPE_W;
}

void channel_disable_read(cdk_channel_t* channel) {
    platform_event_del(channel->poller->pfd, channel->fd, EVENT_TYPE_R, channel);
    channel->events &= ~EVENT_TYPE_R;
}

void channel_disable_all(cdk_channel_t* channel) {
    platform_event_del(channel->poller->pfd, channel->fd, channel->events, channel);
    channel->events = 0;
}

bool channel_is_accepting(cdk_channel_t* channel) {
    return channel->events & EVENT_TYPE_A;
}

bool channel_is_connecting(cdk_channel_t* channel) {
    return channel->events & EVENT_TYPE_C;
}

bool channel_is_writing(cdk_channel_t* channel) {
    return channel->events & EVENT_TYPE_W;
}

bool channel_is_reading(cdk_channel_t* channel) {
    return channel->events & EVENT_TYPE_R;
}

void channel_send_explicit(cdk_channel_t* channel, void* data, size_t size) {
    if (channel->type == SOCK_STREAM) {
        _channel_tcp_send(channel);
    }
    if (channel->type == SOCK_DGRAM) {
        _channel_udp_send(channel);
    }
    ssize_t n = 0;
    if (txlist_empty(&channel->txlist)) {
        if (channel->type == SOCK_STREAM) {
            n = platform_socket_send(channel->fd, data, size);
        }
        if (channel->type == SOCK_DGRAM) {
            n = platform_socket_sendto(channel->fd, data, size, &(channel->udp.peer.ss), channel->udp.peer.sslen);
        }
        if (n == -1) {
            if ((platform_socket_lasterror() == PLATFORM_SO_ERROR_EAGAIN)
                || (platform_socket_lasterror() == PLATFORM_SO_ERROR_EWOULDBLOCK)) {

                txlist_insert(&channel->txlist, data, size, true);
                if (!channel_is_writing(channel)) {
                    channel_enable_write(channel);
                }
                return;
            }
            channel->handler->on_close(channel, platform_socket_error2string(platform_socket_lasterror()));
            return;
        }
    }
    if (n < size) {
        txlist_insert(&channel->txlist, (char*)data + n, (size - n), true);
        if (!channel_is_writing(channel)) {
            channel_enable_write(channel);
        }
    }
    if (n > 0) {
        /**
         * The use of cdk_net_postevent instead of directly invoking on_write is intended
         * to prevent stack overflow.
         */
        cdk_net_postevent(channel->poller, _write_complete_callback, channel, true);
    }
}