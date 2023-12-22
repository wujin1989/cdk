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
#include "cdk/net/cdk-net.h"
#include "cdk/container/cdk-list.h"
#include "channel.h"
#include "txlist.h"
#include "poller.h"
#include "tls.h"
#include "cdk/cdk-time.h"
#include "cdk/cdk-timer.h"
#include "cdk/cdk-utils.h"

cdk_timer_t timer;
static cdk_list_t pollers;
static mtx_t pollers_mtx;
static cnd_t pollers_cnd;
static thrd_t* workers;
static int cnt;

typedef struct async_channel_send_ctx_s{
    cdk_channel_t* channel;
    size_t size;
    char data[];
}async_channel_send_ctx_t;

static void _async_channel_send_explicit_callback(void* param) {
    async_channel_send_ctx_t* ctx = param;

    channel_send_explicit(ctx->channel, ctx->data, ctx->size);

    free(ctx);
    ctx = NULL;
}

static void _async_channel_send_explicit(cdk_channel_t* channel, void* data, size_t size) {
    async_channel_send_ctx_t* ctx = malloc(sizeof(async_channel_send_ctx_t) + size);
    if (ctx) {
        memset(ctx, 0, sizeof(async_channel_send_ctx_t) + size);
        ctx->channel = channel;
        ctx->size = size;
        memcpy(ctx->data, data, size);

        cdk_net_postevent(channel->poller, _async_channel_send_explicit_callback, ctx, true);
    }
}

static void _do_tfo_connect(void* param) {
    cdk_channel_t* channel = param;
    if (channel->tls) {
        channel_tls_cli_handshake(channel);
    } else {
        if (channel_is_connecting(channel)) {
            channel_disable_connect(channel);
        }
        if (channel->handler->on_connect) {
            channel->handler->on_connect(channel);
        }
        if (!channel_is_reading(channel)) {
            channel_enable_read(channel);
        }
    }
}

static void _connect_timeout_callback(void* param) {
    cdk_channel_t* channel = param;
    if (channel->handler->on_close) {
        channel->handler->on_close(channel, platform_socket_error2string(PLATFORM_SO_ERROR_ETIMEDOUT));
    }
}

static void _connect_timeout_routine(void* param) {
    cdk_channel_t* channel = param;
    cdk_net_postevent(channel->poller, _connect_timeout_callback, channel, true);
}

static void _do_accept(void* param) {
    cdk_channel_t* channel = param;
    if (channel->type == SOCK_STREAM) {
        if (!channel_is_accepting(channel)) {
            channel_enable_accept(channel);
        }
    }
    if (channel->type == SOCK_DGRAM) {
        channel_accept(channel);
    }
}

static void _do_connect(void* param) {
    cdk_channel_t* channel = param;
    if (channel->type == SOCK_STREAM) {
        if (!channel_is_connecting(channel)) {
            channel_enable_connect(channel);
        }
        if (channel->handler->connect_timeout) {
            channel->tcp.ctimer = cdk_timer_add(&timer, _connect_timeout_routine, channel, channel->handler->connect_timeout, false);
        }
    }
    if (channel->type == SOCK_DGRAM) {
        channel_connect(channel);
    }
}

static void _async_channel_destroy_callback(void* param) {
    cdk_channel_t* channel = param;
    channel_destroy(channel, "");
}

static void _async_channel_destroy(cdk_channel_t* channel) {
    cdk_net_postevent(channel->poller, _async_channel_destroy_callback, channel, true);
}

static void _inet_ntop(int af, const void* restrict src, char* restrict dst) {
    if (af == AF_INET) {
        inet_ntop(af, src, dst, INET_ADDRSTRLEN);
    }
    if (af == AF_INET6) {
        inet_ntop(af, src, dst, INET6_ADDRSTRLEN);
    }
}

static void _inactive_poller(void* param) {
    cdk_poller_t* poller = param;
    poller->active = false;
}

cdk_poller_t* _poller_roundrobin(void) {
    mtx_lock(&pollers_mtx);
    while (cdk_list_empty(&pollers)) {
        cnd_wait(&pollers_cnd, &pollers_mtx);
    }
    static cdk_poller_t* currpoller;
    if (currpoller == NULL) {
        currpoller = cdk_list_data(cdk_list_head(&pollers), cdk_poller_t, node);
    }
    else {
        cdk_list_node_t* n = cdk_list_next(&currpoller->node);
        if (n != cdk_list_sentinel(&pollers)) {
            currpoller = cdk_list_data(n, cdk_poller_t, node);
        }
        if (n == cdk_list_sentinel(&pollers)) {
            currpoller = cdk_list_data(cdk_list_next(n), cdk_poller_t, node);
        }
    }
    mtx_unlock(&pollers_mtx);
    return currpoller;
}

static int _workerthread(void* param) {
    cdk_poller_t* poller = poller_create();
    if (poller == NULL) {
        return -1;
    }
    mtx_lock(&pollers_mtx);
    cdk_list_insert_tail(&pollers, &poller->node);
    cnd_signal(&pollers_cnd);
    mtx_unlock(&pollers_mtx);

    poller_poll(poller);

    mtx_lock(&pollers_mtx);
    cdk_list_remove(&poller->node);
    mtx_unlock(&pollers_mtx);

    poller_destroy(poller);
    return 0;
}

void cdk_net_ntop(struct sockaddr_storage* ss, cdk_addrinfo_t* ai) {
    char d[INET6_ADDRSTRLEN];
    memset(d, 0, sizeof(d));

    switch (ss->ss_family)
    {
    case AF_INET:
    {
        struct sockaddr_in* si = (struct sockaddr_in*)ss;

        _inet_ntop(AF_INET, &si->sin_addr, d);
        ai->p = ntohs(si->sin_port);
        ai->f = AF_INET;
        break;
    }
    case AF_INET6:
    {
        struct sockaddr_in6* si6 = (struct sockaddr_in6*)ss;

        _inet_ntop(AF_INET6, &si6->sin6_addr, d);
        ai->p = ntohs(si6->sin6_port);
        ai->f = AF_INET6;
        break;
    }
    default:
        return;
    }
    memcpy(ai->a, d, INET6_ADDRSTRLEN);
}

void cdk_net_pton(cdk_addrinfo_t* ai, struct sockaddr_storage* ss) {
    memset(ss, 0, sizeof(struct sockaddr_storage));

    if (ai->f == AF_INET6){
        struct sockaddr_in6* si6 = (struct sockaddr_in6*)ss;

        si6->sin6_family = AF_INET6;
        si6->sin6_port   = htons(ai->p);
        inet_pton(AF_INET6, ai->a, &(si6->sin6_addr));
    }
    if (ai->f == AF_INET){
        struct sockaddr_in* si = (struct sockaddr_in*)ss;

        si->sin_family = AF_INET;
        si->sin_port   = htons(ai->p);
        inet_pton(AF_INET, ai->a, &(si->sin_addr));
    }
}

void cdk_net_obtain_addr(cdk_sock_t sock, cdk_addrinfo_t* ai, bool peer) {
    struct sockaddr_storage ss;
    socklen_t len;

    len = sizeof(struct sockaddr_storage);

    if (peer) {
        getpeername(sock, (struct sockaddr*)&ss, &len);
    }
    if (!peer) {
        getsockname(sock, (struct sockaddr*)&ss, &len);
    }
    cdk_net_ntop(&ss, ai);
}

int cdk_net_af(cdk_sock_t sock) {
    return platform_socket_af(sock);
}

int cdk_net_socktype(cdk_sock_t sock) {
    return platform_socket_socktype(sock);
}

void cdk_net_set_recvbuf(cdk_sock_t sock, int val) {
    platform_socket_set_recvbuf(sock, val);
}

void cdk_net_set_sendbuf(cdk_sock_t sock, int val) {
    platform_socket_set_sendbuf(sock, val);
}

void cdk_net_listen(cdk_protocol_t protocol, const char* host, const char* port, cdk_handler_t* handler) {
    int proto = 0;
    if (protocol == PROTOCOL_TCP) {
        proto = SOCK_STREAM;
    }
    if (protocol == PROTOCOL_UDP) {
        proto = SOCK_DGRAM;
    }
    cdk_sock_t sock = platform_socket_listen(host, port, proto);
    cdk_channel_t* channel = channel_create(_poller_roundrobin(), sock, handler);
    if (channel) {
        cdk_net_postevent(channel->poller, _do_accept, channel, true);
    }
}

void cdk_net_dial(cdk_protocol_t protocol, const char* host, const char* port, cdk_handler_t* handler) {
    int proto = 0;
    bool connected = false;
    
    if (protocol == PROTOCOL_TCP) {
        proto = SOCK_STREAM;
    }
    if (protocol == PROTOCOL_UDP) {
        proto = SOCK_DGRAM;
    }
    cdk_sock_t sock = platform_socket_dial(host, port, proto, &connected);
    cdk_channel_t* channel = channel_create(_poller_roundrobin(), sock, handler);
    if (channel) {
        if (channel->type == SOCK_STREAM) {
            if (connected) {
                cdk_net_postevent(channel->poller, _do_tfo_connect, channel, true);
            } else {
                cdk_net_postevent(channel->poller, _do_connect, channel, true);
            }
        }
        if (channel->type == SOCK_DGRAM) {
            getpeername(sock, (struct sockaddr*)&channel->udp.peeraddr, &channel->udp.peerlen);
            cdk_net_postevent(channel->poller, _do_connect, channel, true);
        }
    }
}

void cdk_net_send(cdk_channel_t* channel, void* data, size_t size) {
	if (thrd_equal(channel->poller->tid, thrd_current())) {
        channel_send_explicit(channel, data, size);
    }
    else {
        _async_channel_send_explicit(channel, data, size);
    }
}

void cdk_net_close(cdk_channel_t* channel) {
    if (thrd_equal(channel->poller->tid, thrd_current())) {
        channel_destroy(channel, NULL);
    }
    else {
        _async_channel_destroy(channel);
    }
}

void cdk_net_postevent(cdk_poller_t* poller, void (*cb)(void*), void* arg, bool totail) {
    cdk_event_t* event = malloc(sizeof(cdk_event_t));
    if (event) {
        event->cb = cb;
        event->arg = arg;

        mtx_lock(&poller->evmtx);
        if (totail) {
            cdk_list_insert_tail(&poller->evlist, &event->node);
        }
        else {
            cdk_list_insert_head(&poller->evlist, &event->node);
        }
        mtx_unlock(&poller->evmtx);

        int hardcode = 1;
        platform_socket_send(poller->evfds[0], &hardcode, sizeof(int));
    }
}

void cdk_net_stop(void) {
    mtx_lock(&pollers_mtx);
    for (cdk_list_node_t* n = cdk_list_head(&pollers); n != cdk_list_sentinel(&pollers); n = cdk_list_next(n)) {
        cdk_poller_t* poller = cdk_list_data(n, cdk_poller_t, node);
        cdk_net_postevent(poller, _inactive_poller, poller, true);
    }
    mtx_unlock(&pollers_mtx);
}

void cdk_net_startup(int nworkers) {
    platform_socket_startup();

    if (!nworkers) {
        cnt = 1;
    }
    else {
        cnt = nworkers;
    }
    cdk_timer_create(&timer, 1);
    cdk_list_init(&pollers);
    mtx_init(&pollers_mtx, mtx_plain);
    cnd_init(&pollers_cnd);

    workers = malloc(cnt * sizeof(thrd_t));
    for (int i = 0; i < cnt; i++) {
        thrd_create(workers + i, _workerthread, NULL);
    }
}

void cdk_net_cleanup(void) {
    for(int i = 0; i < cnt; i++) {
        thrd_join(workers[i], NULL);
    }
    free(workers);
    workers = NULL;
    
    platform_socket_cleanup();
    cdk_timer_destroy(&timer);
    mtx_destroy(&pollers_mtx);
    cnd_destroy(&pollers_cnd);
}
