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
#include "unpacker.h"
#include "tls.h"
#include "txlist.h"
#include "cdk/cdk-timer.h"
#include "cdk/cdk-time.h"
#include "cdk/net/cdk-net.h"
#include "cdk/container/cdk-rbtree.h"

#define SSL_CLEANUP_PERIOD (60 * 1000) //60s

extern cdk_poller_manager_t global_poller_manager;
extern cdk_tls_ctx_t* global_tls_ctx;
extern cdk_tls_ctx_t* global_dtls_ctx;

static inline void _dtls_sslmap_cleanup(cdk_channel_t* channel) {
    if (channel->udp.sslmap) {
        while (!cdk_rbtree_empty(channel->udp.sslmap)) {
            cdk_ssl_entry_t* entry = cdk_rbtree_data(cdk_rbtree_first(channel->udp.sslmap), cdk_ssl_entry_t, node);
            cdk_rbtree_erase(channel->udp.sslmap, &entry->node);

            tls_ssl_destroy(entry->ssl);
            free(entry->node.key.ptr);
            entry->node.key.ptr = NULL;

            free(entry);
            entry = NULL;
        }
        free(channel->udp.sslmap);
        channel->udp.sslmap = NULL;
    }
}

static inline void _cb_write_complete(void* param) {
    cdk_channel_t* channel = param;
    
    if (channel->type == SOCK_STREAM) {
        channel->handler->tcp.on_write(channel);
    }
    if (channel->type == SOCK_DGRAM) {
        channel->handler->udp.on_write(channel);
    }
}

static void _unencrypted_send(cdk_channel_t* channel) {
    if (txlist_empty(&channel->txlist)) {
        if (channel_is_writing(channel)) {
            channel_disable_write(channel);
        }
        return;
    }
    ssize_t n = 0;
    txlist_node_t* e = cdk_list_data(cdk_list_head(&(channel->txlist)), txlist_node_t, n);
    if (channel->type == SOCK_STREAM) {
        n = platform_socket_send(channel->fd, e->buf, (int)e->len);
    }
    if (channel->type == SOCK_DGRAM) {
        n = platform_socket_sendto(channel->fd, e->buf, (int)e->len, &(channel->udp.peer.ss), channel->udp.peer.sslen);
    }
    if (n == PLATFORM_SO_ERROR_SOCKET_ERROR) {
        if ((platform_socket_lasterror() == PLATFORM_SO_ERROR_EAGAIN)
            || (platform_socket_lasterror() == PLATFORM_SO_ERROR_EWOULDBLOCK)) {
            return;
        }
        channel_destroy(channel, platform_socket_error2string(platform_socket_lasterror()));
        return;
    }
    if (n < e->len) {
        txlist_insert(&channel->txlist, e->buf + n, (e->len - n), false);
    }
    if (channel->type == SOCK_STREAM) {
        channel->tcp.latest_wr_time = cdk_time_now();
        if (n > 0 && channel->handler->tcp.on_write) {
            channel->handler->tcp.on_write(channel);
        }
    }
    if (channel->type == SOCK_DGRAM) {
        if (n > 0 && channel->handler->udp.on_write) {
            channel->handler->udp.on_write(channel);
        }
    }
    txlist_remove(e);
}

static void _encrypted_send(cdk_channel_t* channel) {
    int err = 0;
    if (txlist_empty(&channel->txlist)) {
        if (channel_is_writing(channel)) {
            channel_disable_write(channel);
        }
        return;
    }
    txlist_node_t* e = cdk_list_data(cdk_list_head(&(channel->txlist)), txlist_node_t, n);
    int n = tls_ssl_write((channel->type == SOCK_STREAM) ? channel->tcp.ssl : channel->udp.active_ssl->ssl, e->buf, (int)e->len, &err);
    if (n <= 0) {
        if (n == 0) {
            return;
        }
        channel_destroy(channel, tls_ssl_error2string(err));
        return;
    }
    if (n < e->len) {
        txlist_insert(&channel->txlist, e->buf + n, (e->len - n), false);
    }
    if (channel->type == SOCK_STREAM) {
        channel->tcp.latest_wr_time = cdk_time_now();
        if (channel->handler->tcp.on_write) {
            channel->handler->tcp.on_write(channel);
        }
    }
    if (channel->type == SOCK_DGRAM) {
        channel->udp.active_ssl->latest_wr_time = cdk_time_now();
        if (channel->handler->udp.on_write) {
            channel->handler->udp.on_write(channel);
        }
    }
    txlist_remove(e);
}

static void _encrypted_send_explicit(cdk_channel_t* channel, void* data, size_t size) {
    int n = 0;
    if (txlist_empty(&channel->txlist)) {
        int err = 0;
        n = tls_ssl_write((channel->type == SOCK_STREAM) ? channel->tcp.ssl : channel->udp.active_ssl->ssl, data, size, &err);
        if (n <= 0) {
            if (n == 0) {
                txlist_insert(&channel->txlist, data, size, true);
                if (!channel_is_writing(channel)) {
                    channel_enable_write(channel);
                }
                return;
            }
            channel_destroy(channel, tls_ssl_error2string(err));
            return;
        }
    }
    if (n < size) {
        txlist_insert(&channel->txlist, (char*)data + n, (size - n), true);
        if (!channel_is_writing(channel)) {
            channel_enable_write(channel);
        }
    }
    if (channel->type == SOCK_STREAM) {
        channel->tcp.latest_wr_time = cdk_time_now();
        if (n > 0 && channel->handler->tcp.on_write) {
            cdk_net_postevent(channel->poller, _cb_write_complete, channel, true);
        }
    }
    if (channel->type == SOCK_DGRAM) {
        channel->udp.active_ssl->latest_wr_time = cdk_time_now();
        if (n > 0 && channel->handler->udp.on_write) {
            cdk_net_postevent(channel->poller, _cb_write_complete, channel, true);
        }
    }
}

static void _unencrypted_send_explicit(cdk_channel_t* channel, void* data, size_t size) {
    ssize_t n = 0;
    if (txlist_empty(&channel->txlist)) {
        if (channel->type == SOCK_STREAM) {
            n = platform_socket_send(channel->fd, data, size);
        }
        if (channel->type == SOCK_DGRAM) {
            n = platform_socket_sendto(channel->fd, data, size, &(channel->udp.peer.ss), channel->udp.peer.sslen);
        }
        if (n == PLATFORM_SO_ERROR_SOCKET_ERROR) {
            if ((platform_socket_lasterror() == PLATFORM_SO_ERROR_EAGAIN)
                || (platform_socket_lasterror() == PLATFORM_SO_ERROR_EWOULDBLOCK)) {

                txlist_insert(&channel->txlist, data, size, true);
                if (!channel_is_writing(channel)) {
                    channel_enable_write(channel);
                }
                return;
            }
            channel_destroy(channel, platform_socket_error2string(platform_socket_lasterror()));
            return;
        }
    }
    if (n < size) {
        txlist_insert(&channel->txlist, (char*)data + n, (size - n), true);
        if (!channel_is_writing(channel)) {
            channel_enable_write(channel);
        }
    }
    if (channel->type == SOCK_STREAM) {
        channel->tcp.latest_wr_time = cdk_time_now();
        if (n > 0 && channel->handler->tcp.on_write) {
            cdk_net_postevent(channel->poller, _cb_write_complete, channel, true);
        }
    }
    if (channel->type == SOCK_DGRAM) {
        if (n > 0 && channel->handler->udp.on_write) {
            cdk_net_postevent(channel->poller, _cb_write_complete, channel, true);
        }
    }
}

static void _encrypted_recv(cdk_channel_t* channel) {
    int err = 0;
    int n = 0;
    if (channel->type == SOCK_STREAM) {
        n = tls_ssl_read(channel->tcp.ssl, (char*)(channel->rxbuf.buf) + channel->rxbuf.off, DEFAULT_IOBUF_SIZE / 2, &err);
    }
    if (channel->type == SOCK_DGRAM) {
        n = tls_ssl_read(channel->udp.active_ssl->ssl, channel->rxbuf.buf, DEFAULT_IOBUF_SIZE, &err);
    }
    if (n <= 0) {
        if (n == 0) {
            return;
        }
        channel_destroy(channel, tls_ssl_error2string(err));
        return;
    }
    if (channel->type == SOCK_STREAM) {
        channel->tcp.latest_rd_time = cdk_time_now();
        channel->rxbuf.off += n;
        unpacker_unpack(channel);
    }
    if (channel->type == SOCK_DGRAM) {
        channel->udp.active_ssl->latest_rd_time = cdk_time_now();
        if (channel->handler->udp.on_read) {
            channel->handler->udp.on_read(channel, channel->rxbuf.buf, n);
        }
    }
}

static void _unencrypted_recv(cdk_channel_t* channel) {
    ssize_t n;
    if (channel->type == SOCK_STREAM) {
        n = platform_socket_recv(channel->fd, (char*)(channel->rxbuf.buf) + channel->rxbuf.off, DEFAULT_IOBUF_SIZE / 2);
    }
    if (channel->type == SOCK_DGRAM) {
        channel->udp.peer.sslen = sizeof(struct sockaddr_storage);
        n = platform_socket_recvfrom(channel->fd, channel->rxbuf.buf, DEFAULT_IOBUF_SIZE, &channel->udp.peer.ss, &channel->udp.peer.sslen);
    }
    if (n == PLATFORM_SO_ERROR_SOCKET_ERROR) {
        if ((platform_socket_lasterror() == PLATFORM_SO_ERROR_EAGAIN)
            || (platform_socket_lasterror() == PLATFORM_SO_ERROR_EWOULDBLOCK)) {
            return;
        }
        channel_destroy(channel, platform_socket_error2string(platform_socket_lasterror()));
        return;
    }
    if (n == 0) {
        channel_destroy(channel, platform_socket_error2string(PLATFORM_SO_ERROR_ECONNRESET));
        return;
    }
    if (channel->type == SOCK_STREAM) {
        channel->tcp.latest_rd_time = cdk_time_now();
        channel->rxbuf.off += n;
        unpacker_unpack(channel);
    }
    if (channel->type == SOCK_DGRAM) {
        if (channel->handler->udp.on_read) {
            channel->handler->udp.on_read(channel, channel->rxbuf.buf, n);
        }
    }
}

static inline void _channel_destroy_cb(void* param) {
    cdk_channel_t* channel = param;
    if (channel) {
        free(channel);
        channel = NULL;
    }
}

static void _rd_timeout_cb(void* param) {
    cdk_channel_t* channel = param;
    if ((cdk_time_now() - channel->tcp.latest_rd_time) > channel->handler->tcp.rd_timeout) {
        cdk_net_close(channel);
    }
}

static void _wr_timeout_cb(void* param) {
    cdk_channel_t* channel = param;
    if ((cdk_time_now() - channel->tcp.latest_wr_time) > channel->handler->tcp.wr_timeout) {
        cdk_net_close(channel);
    }
}

static void _conn_timeout_cb(void* param) {
    cdk_channel_t* channel = param;
    cdk_net_close(channel);
}

static void _ssl_timeout_cb(void* param) {
    cdk_channel_t* channel = param;
    
    for(cdk_rbtree_node_t* n = cdk_rbtree_first(channel->udp.sslmap); n != cdk_rbtree_last(channel->udp.sslmap); ){
        cdk_ssl_entry_t* entry = cdk_rbtree_data(n, cdk_ssl_entry_t, node);
        n = cdk_rbtree_next(n);
        
        if ((cdk_time_now() - entry->latest_rd_time) < SSL_CLEANUP_PERIOD) {
            continue;
        }
        if ((cdk_time_now() - entry->latest_wr_time) < SSL_CLEANUP_PERIOD) {
            continue;
        }
        cdk_rbtree_erase(channel->udp.sslmap, &entry->node);
        
        tls_ssl_destroy(entry->ssl);
        free(entry->node.key.ptr);
        entry->node.key.ptr = NULL;

        free(entry);
        entry = NULL;
    }
}

void channel_connecting(cdk_channel_t* channel) {
    channel->tcp.connecting = true;
    if (!channel_is_writing(channel)) {
        channel_enable_write(channel);
    }
    if (channel->handler->tcp.conn_timeout) {
        channel->tcp.conn_timer = cdk_timer_add(&channel->poller->timermgr, _conn_timeout_cb, channel, channel->handler->tcp.conn_timeout, false);
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
        channel->tcp.hb_timer = cdk_timer_add(&channel->poller->timermgr, _heartbeat_cb, channel, channel->handler->tcp.hb_interval, true);
    }
    if (channel->handler->tcp.rd_timeout) {
        channel->tcp.rd_timer = cdk_timer_add(&channel->poller->timermgr, _rd_timeout_cb, channel, channel->handler->tcp.rd_timeout, true);
    }
    if (channel->handler->tcp.wr_timeout) {
        channel->tcp.wr_timer = cdk_timer_add(&channel->poller->timermgr, _wr_timeout_cb, channel, channel->handler->tcp.wr_timeout, true);
    }
}

static void _tcp_timers_destroy(cdk_channel_t* channel) {
    if (channel->tcp.hb_timer) {
        cdk_timer_del(&channel->poller->timermgr, channel->tcp.hb_timer);
    }
    if (channel->tcp.rd_timer) {
        cdk_timer_del(&channel->poller->timermgr, channel->tcp.rd_timer);
    }
    if (channel->tcp.wr_timer) {
        cdk_timer_del(&channel->poller->timermgr, channel->tcp.wr_timer);
    }
    if (channel->tcp.conn_timer) {
        cdk_timer_del(&channel->poller->timermgr, channel->tcp.conn_timer);
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
        if (channel->tcp.ssl) {
            _encrypted_recv(channel);
        } else {
            _unencrypted_recv(channel);
        }
    }
    if (channel->type == SOCK_DGRAM) {
        if (channel->udp.active_ssl) {
            _encrypted_recv(channel);
        } else {
            _unencrypted_recv(channel);
        }
    }
}

void channel_accepted(cdk_channel_t* channel) {
    if (channel->type == SOCK_STREAM) {
        if (channel->handler->tcp.on_accept) {
            channel->handler->tcp.on_accept(channel);
        }
        if (!channel_is_reading(channel)) {
            channel_enable_read(channel);
        }
        _tcp_timers_create(channel);
    }
    if (channel->type == SOCK_DGRAM) {
        channel_recv(channel);
    }
}

void channel_accepting(cdk_channel_t* channel) {
    if (channel->type == SOCK_STREAM) {
        channel->tcp.accepting = true;
    }
    if (channel->type == SOCK_DGRAM) {
        channel->udp.accepting = true;
    }
    if (!channel_is_reading(channel)) {
        channel_enable_read(channel);
    }
}

void channel_tls_cli_handshake(void* param) {
    int err = 0;
    cdk_channel_t* channel = param;

    int n = tls_ssl_connect((channel->type == SOCK_STREAM) ? channel->tcp.ssl : channel->udp.active_ssl->ssl, channel->fd, &err);
    if (n <= 0) {
        if (n == 0) {
            cdk_net_postevent(channel->poller, channel_tls_cli_handshake, channel, true);
            return;
        }
        channel_destroy(channel, tls_ssl_error2string(err));
        return;
    }
    channel_connected(channel);
}

void channel_tls_srv_handshake(void* param) {
    int err = 0;
    int n = 0;
    cdk_channel_t* channel = param;
    if (channel->type == SOCK_STREAM) {
        n = tls_ssl_accept(channel->tcp.ssl, channel->fd, &err);
    }
    if (channel->type == SOCK_DGRAM) {
        cdk_tls_ssl_t* ssl = tls_ssl_create(global_dtls_ctx);
        n = tls_ssl_dtls_listen(ssl, channel->fd, &err);
        if (n > 0) {
            cdk_ssl_entry_t* entry = malloc(sizeof(cdk_ssl_entry_t));
            if (entry) {
                entry->ssl = ssl;
                entry->latest_rd_time = 0;
                entry->latest_wr_time = 0;
                entry->node.key.str = malloc(sizeof(channel->udp.peer_human));
                if (entry->node.key.str) {
                    memcpy(entry->node.key.str, channel->udp.peer_human, sizeof(channel->udp.peer_human));
                }
                channel->udp.active_ssl = entry;
                cdk_rbtree_insert(channel->udp.sslmap, &entry->node);
            }
        }
    }
	if (n <= 0) {
		if (n == 0) {
			cdk_net_postevent(channel->poller, channel_tls_srv_handshake, channel, true);
			return;
		}
		channel_destroy(channel, tls_ssl_error2string(err));
		return;
	}
    channel_accepted(channel);
}

cdk_channel_t* channel_create(cdk_poller_t* poller, cdk_sock_t sock, cdk_handler_t* handler)
{
    cdk_channel_t* channel = malloc(sizeof(cdk_channel_t));

    if (channel) {
        memset(channel, 0, sizeof(cdk_channel_t));
        channel->poller = poller;
        channel->fd = sock;
        channel->handler = handler;
        channel->type = platform_socket_getsocktype(sock);
        atomic_init(&channel->closing, false);
        
        channel->rxbuf.len = DEFAULT_IOBUF_SIZE;
        channel->rxbuf.off = 0;
        channel->rxbuf.buf = malloc(DEFAULT_IOBUF_SIZE);
        if (channel->rxbuf.buf) {
            memset(channel->rxbuf.buf, 0, DEFAULT_IOBUF_SIZE);
        }
        txlist_create(&channel->txlist);
        if (channel->type == SOCK_STREAM) {
            if (global_tls_ctx) {
                channel->tcp.ssl = tls_ssl_create(global_tls_ctx);
            }
        }
        cdk_list_insert_tail(&poller->chlist, &channel->node);
        return channel;
    }
    return NULL;
}

void channel_destroy(cdk_channel_t* channel, const char* reason) {
    if (atomic_load(&channel->closing)) {
        return;
    }
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
        tls_ssl_destroy(channel->tcp.ssl);
        if (channel->handler->tcp.on_close) {
            channel->handler->tcp.on_close(channel, reason);
        }
        _tcp_timers_destroy(channel);
    }
    if (channel->type == SOCK_DGRAM) {
        _dtls_sslmap_cleanup(channel);
        if (channel->handler->udp.on_close) {
            channel->handler->udp.on_close(channel, reason);
        }
    }
    if (channel->poller->active) {
        cdk_net_postevent(channel->poller, _channel_destroy_cb, channel, true);
    } else {
        if (channel) {
            free(channel);
            channel = NULL;
        }
    }
}

void channel_send(cdk_channel_t* channel) {
    if (channel->type == SOCK_STREAM) {
        if (channel->tcp.ssl) {
            _encrypted_send(channel);
        } else {
            _unencrypted_send(channel);
        }
    }
    if (channel->type == SOCK_DGRAM) {
        if (channel->udp.active_ssl) {
            _encrypted_send(channel);
        } else {
            _unencrypted_send(channel);
        }
    }
}

void channel_accept(cdk_channel_t* channel) {
    if (channel->type == SOCK_STREAM) {
        cdk_sock_t cli = platform_socket_accept(channel->fd, true);
        if (cli == PLATFORM_SO_ERROR_INVALID_SOCKET) {
            if ((platform_socket_lasterror() == PLATFORM_SO_ERROR_EAGAIN)
                || (platform_socket_lasterror() == PLATFORM_SO_ERROR_EWOULDBLOCK)) {
                return;
            }
            channel_destroy(channel, platform_socket_error2string(platform_socket_lasterror()));
            return;
        }
        cdk_channel_t* nchannel = channel_create(global_poller_manager.poller_roundrobin(), cli, channel->handler);
        if (nchannel) {
            if (nchannel->tcp.ssl) {
                channel_tls_srv_handshake(nchannel);
            } else {
                channel_accepted(nchannel);
            }
        }
    }
    if (channel->type == SOCK_DGRAM) {
        if(global_dtls_ctx){
            if(!channel->udp.sslmap){
                channel->udp.sslmap = malloc(sizeof(cdk_rbtree_t));
                if (channel->udp.sslmap) {
                    cdk_rbtree_init(channel->udp.sslmap, default_keycmp_str);
                }
            }
            if(!channel->udp.ssl_cleanup_timer){
                channel->udp.ssl_cleanup_timer =
                cdk_timer_add(&channel->poller->timermgr, _ssl_timeout_cb, channel, SSL_CLEANUP_PERIOD, true);
            }
            channel->udp.peer.sslen = sizeof(struct sockaddr_storage);
            recvfrom(channel->fd,
                     channel->rxbuf.buf,
                     DEFAULT_IOBUF_SIZE, MSG_PEEK,
                     (struct sockaddr*)&channel->udp.peer.ss,
                     &channel->udp.peer.sslen);
            
            cdk_address_t addrinfo = { 0 };
            cdk_net_ntop(&channel->udp.peer.ss, &addrinfo);
            sprintf(channel->udp.peer_human, "%s:%d", addrinfo.addr, addrinfo.port);
            
            cdk_rbtree_key_t key = { .str = channel->udp.peer_human };
            cdk_rbtree_node_t* n = cdk_rbtree_find(channel->udp.sslmap, key);
            if (!n) {
                channel_tls_srv_handshake(channel);
            } else {
                cdk_ssl_entry_t* entry = cdk_rbtree_data(n, cdk_ssl_entry_t, node);
                channel->udp.active_ssl = entry;
                channel_recv(channel);
            }
        }else{
            channel_recv(channel);

        }
    }
}

void channel_connect(cdk_channel_t* channel) {
    if (atomic_load(&channel->closing)) {
        return;
    }
    int err = 0;
    socklen_t len = sizeof(int);

    channel_disable_all(channel);
    if (channel->handler->tcp.conn_timeout) {
        cdk_timer_del(&channel->poller->timermgr, channel->tcp.conn_timer);
    }
    getsockopt(channel->fd, SOL_SOCKET, SO_ERROR, (char*)&err, &len);
    if (err) {
        channel_destroy(channel, platform_socket_error2string(err));
    } else {
        if (channel->tcp.ssl) {
            channel_tls_cli_handshake(channel);
        } else {
            channel_connected(channel);
        }
    }
}

void channel_enable_write(cdk_channel_t* channel) {
    if (channel->events) {
        platform_event_mod(channel->poller->pfd, channel->fd, channel->events | EVENT_WR, channel);
    } else {
        platform_event_add(channel->poller->pfd, channel->fd, channel->events | EVENT_WR, channel);
    }
    channel->events |= EVENT_WR;
}

void channel_enable_read(cdk_channel_t* channel) {
    if (channel->events) {
        platform_event_mod(channel->poller->pfd, channel->fd, channel->events | EVENT_RD, channel);
    } else {
        platform_event_add(channel->poller->pfd, channel->fd, channel->events | EVENT_RD, channel);
    }
    channel->events |= EVENT_RD;
}

void channel_disable_write(cdk_channel_t* channel) {
    if (channel->events) {
        platform_event_mod(channel->poller->pfd, channel->fd, channel->events & ~EVENT_WR, channel);
    } else {
        platform_event_del(channel->poller->pfd, channel->fd);
    }
    channel->events &= ~EVENT_WR;
}

void channel_disable_read(cdk_channel_t* channel) {
    if (channel->events) {
        platform_event_mod(channel->poller->pfd, channel->fd, channel->events & ~EVENT_RD, channel);
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

void channel_send_explicit(cdk_channel_t* channel, void* data, size_t size) {
    if (atomic_load(&channel->closing)) {
        return;
    }
    if (channel->type == SOCK_STREAM) {
        if (channel->tcp.ssl) {
            _encrypted_send_explicit(channel, data, size);
        } else {
            _unencrypted_send_explicit(channel, data, size);
        }
    }
    if (channel->type == SOCK_DGRAM) {
        if (channel->udp.active_ssl) {
            _encrypted_send_explicit(channel, data, size);
        } else {
            _unencrypted_send_explicit(channel, data, size);
        }
    }
}
