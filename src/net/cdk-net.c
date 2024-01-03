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
#include "cdk/cdk-logger.h"

cdk_poller_manager_t manager = {.initialized = ATOMIC_FLAG_INIT };

typedef struct async_channel_send_ctx_s{
    cdk_channel_t* channel;
    size_t size;
    char data[];
}async_channel_send_ctx_t;

typedef struct async_socket_ctx_s {
    char host[INET6_ADDRSTRLEN];
    char port[6];
    int protocol;
    int idx;
    int cores;
    cdk_poller_t* poller;
    cdk_handler_t* handler;
}async_socket_ctx_t;

static void _poller_inactive_cb(void* param) {
    cdk_poller_t* poller = param;
    poller->active = false;
}

static cdk_poller_t* _poller_roundrobin(void) {
    mtx_lock(&manager.poller_mtx);
    while (cdk_list_empty(&manager.poller_lst)) {
        cnd_wait(&manager.poller_cnd, &manager.poller_mtx);
    }
    static cdk_poller_t* curr;
    if (curr == NULL) {
        curr = cdk_list_data(cdk_list_head(&manager.poller_lst), cdk_poller_t, node);
    } else {
        cdk_list_node_t* n = cdk_list_next(&curr->node);
        if (n != cdk_list_sentinel(&manager.poller_lst)) {
            curr = cdk_list_data(n, cdk_poller_t, node);
        } else {
            curr = cdk_list_data(cdk_list_next(n), cdk_poller_t, node);
        }
    }
    mtx_unlock(&manager.poller_mtx);
    return curr;
}

static void _poller_manager_add(cdk_poller_t* poller) {
    mtx_lock(&manager.poller_mtx);
    cdk_list_insert_tail(&manager.poller_lst, &poller->node);
    cnd_signal(&manager.poller_cnd);
    mtx_unlock(&manager.poller_mtx);
}

static void _poller_manager_del(cdk_poller_t* poller) {
    mtx_lock(&manager.poller_mtx);
    cdk_list_remove(&poller->node);
    mtx_unlock(&manager.poller_mtx);
}

static int _poller_routine(void* param) {
    cdk_poller_t* poller = poller_create();
    if (poller) {
        _poller_manager_add(poller);
        poller_poll(poller);
        _poller_manager_del(poller);

        poller_destroy(poller);
    }
    return 0;
}

static void _poller_manager_create(int parallel) {
    platform_socket_startup();
    if (parallel <= 0) {
        abort();
    }
    cdk_timer_create();
    cdk_list_init(&manager.poller_lst);
    mtx_init(&manager.poller_mtx, mtx_plain);
    cnd_init(&manager.poller_cnd);

    manager.poller_roundrobin = _poller_roundrobin;
    manager.thrdcnt = parallel;
    manager.thrdids = malloc(manager.thrdcnt * sizeof(thrd_t));
    for (int i = 0; i < manager.thrdcnt; i++) {
        thrd_create((manager.thrdids + i), _poller_routine, NULL);
    }
}

static void _poller_manager_destroy(void) {
    for (int i = 0; i < manager.thrdcnt; i++) {
        thrd_join(manager.thrdids[i], NULL);
    }
    free(manager.thrdids);
    manager.thrdids = NULL;
    
    cdk_timer_destroy();
    mtx_destroy(&manager.poller_mtx);
    cnd_destroy(&manager.poller_cnd);
    platform_socket_cleanup();
}

static async_socket_ctx_t* _allocate_async_socket_ctx(const char* protocol, const char* host, const char* port, int idx, int cores, cdk_handler_t* handler) {
    async_socket_ctx_t* ctx = malloc(sizeof(async_socket_ctx_t));
    if (ctx) {
        memset(ctx, 0, sizeof(async_socket_ctx_t));
        memcpy(ctx->host, host, strlen(host));
        memcpy(ctx->port, port, strlen(port));
        if (!strcmp(protocol, "tcp")) {
            ctx->protocol = SOCK_STREAM;
        }
        if (!strcmp(protocol, "udp")) {
            ctx->protocol = SOCK_DGRAM;
        }
        ctx->idx = idx;
        ctx->cores = cores;
        ctx->handler = handler;
        ctx->poller = manager.poller_roundrobin();
    }
    return ctx;
}

static void _async_listen_cb(void* param) {
    async_socket_ctx_t* ctx = param;

    cdk_sock_t sock = platform_socket_listen(ctx->host, ctx->port, ctx->protocol, ctx->idx, ctx->cores, true);
    cdk_channel_t* channel = channel_create(ctx->poller, sock, ctx->handler);
    if (channel) {
        if (channel->type == SOCK_STREAM) {
            channel_accepting(channel);
        }
        if (channel->type == SOCK_DGRAM) {
            if (!channel_is_reading(channel)) {
                channel_enable_read(channel);
            }
        }
    }
    free(ctx);
    ctx = NULL;
}

static void _async_dial_cb(void* param) {
    async_socket_ctx_t* ctx = param;
    bool connected = false;

    cdk_sock_t sock = platform_socket_dial(ctx->host, ctx->port, ctx->protocol, &connected, true);
    cdk_channel_t* channel = channel_create(ctx->poller, sock, ctx->handler);
    if (channel) {
        if (channel->type == SOCK_STREAM) {
            if (connected) {
                if (channel->tcp.tls) {
                    channel_tls_cli_handshake(channel);
                } else {
                    channel_connected(channel);
                }
            } else {
                channel_connecting(channel);
            }
        }
        if (channel->type == SOCK_DGRAM) {
            cdk_addrinfo_t ai = { 0 };
            
            cdk_net_getaddrinfo(sock, &ai, true);
            cdk_net_pton(&ai, &channel->udp.peer.ss);
            channel->udp.peer.sslen = (ai.f == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);

            channel_connected(channel);
        }
    }
    free(ctx);
    ctx = NULL;
}

static void _async_channel_send_explicit_cb(void* param) {
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

        cdk_net_postevent(channel->poller, _async_channel_send_explicit_cb, ctx, true);
    }
}

static void _async_channel_destroy_cb(void* param) {
    cdk_channel_t* channel = param;
    channel_destroy(channel, "");
}

static void _async_channel_destroy(cdk_channel_t* channel) {
    cdk_net_postevent(channel->poller, _async_channel_destroy_cb, channel, true);
}

static void _inet_ntop(int af, const void* restrict src, char* restrict dst) {
    if (af == AF_INET) {
        inet_ntop(af, src, dst, INET_ADDRSTRLEN);
    }
    if (af == AF_INET6) {
        inet_ntop(af, src, dst, INET6_ADDRSTRLEN);
    }
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

void cdk_net_getaddrinfo(cdk_sock_t sock, cdk_addrinfo_t* ai, bool peer) {
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

int cdk_net_getsocktype(cdk_sock_t sock) {
    return platform_socket_getsocktype(sock);
}

void cdk_net_listen(const char* protocol, const char* host, const char* port, cdk_handler_t* handler) {
    for (int i = 0; i < manager.thrdcnt; i++) {
        async_socket_ctx_t* ctx = _allocate_async_socket_ctx(protocol, host, port, i, manager.thrdcnt, handler);
        if (ctx) {
            cdk_net_postevent(ctx->poller, _async_listen_cb, ctx, true);
        }
    }
}

void cdk_net_dial(const char* protocol, const char* host, const char* port, cdk_handler_t* handler) {
    async_socket_ctx_t* ctx = _allocate_async_socket_ctx(protocol, host, port, 0, 0, handler);
    if (ctx) {
        cdk_net_postevent(ctx->poller, _async_dial_cb, ctx, true);
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
        } else {
            cdk_list_insert_head(&poller->evlist, &event->node);
        }
        mtx_unlock(&poller->evmtx);

        int hardcode = 1;
        platform_socket_sendall(poller->evfds[0], &hardcode, sizeof(int));
    }
}

void cdk_net_exit(void) {
    mtx_lock(&manager.poller_mtx);
    for (cdk_list_node_t* n = cdk_list_head(&manager.poller_lst); n != cdk_list_sentinel(&manager.poller_lst); n = cdk_list_next(n)) {
        cdk_poller_t* poller = cdk_list_data(n, cdk_poller_t, node);
        cdk_net_postevent(poller, _poller_inactive_cb, poller, true);
    }
    mtx_unlock(&manager.poller_mtx);
}

void cdk_net_startup(int nthrds) {
    if (atomic_flag_test_and_set(&manager.initialized)) {
        return;
    }
    _poller_manager_create(nthrds);
}

void cdk_net_cleanup(void) {
    _poller_manager_destroy();
}

void cdk_net_startup2(void) {
    platform_socket_startup();
}

void cdk_net_cleanup2(void) {
    platform_socket_cleanup();
}

cdk_sock_t cdk_net_listen2(const char* protocol, const char* host, const char* port){
    int proto = 0;
    if (!strcmp(protocol, "tcp")) {
        proto = SOCK_STREAM;
    }
    if (!strcmp(protocol, "udp")) {
        proto = SOCK_DGRAM;
    }
    return platform_socket_listen(host, port, proto, 0, 0, false);
}

cdk_sock_t cdk_net_accept2(cdk_sock_t sock){
    return platform_socket_accept(sock, false);
}

cdk_sock_t cdk_net_dial2(const char* protocol, const char* host, const char* port){
    int proto = 0;
    bool unused;
    if (!strcmp(protocol, "tcp")) {
        proto = SOCK_STREAM;
    }
    if (!strcmp(protocol, "udp")) {
        proto = SOCK_DGRAM;
    }
    return platform_socket_dial(host, port, proto, &unused, false);
}

ssize_t cdk_net_recv2(cdk_sock_t sock, void* buf, int size){
    return platform_socket_recv(sock, buf, size);
}

ssize_t cdk_net_send2(cdk_sock_t sock, void* buf, int size){
    return platform_socket_send(sock, buf, size);
}

ssize_t cdk_net_recvall2(cdk_sock_t sock, void* buf, int size) {
    return platform_socket_recvall(sock, buf, size);
}

ssize_t cdk_net_sendall2(cdk_sock_t sock, void* buf, int size) {
    return platform_socket_sendall(sock, buf, size);
}

ssize_t cdk_net_recvfrom2(cdk_sock_t sock, void* buf, int size, struct sockaddr_storage* ss, socklen_t* sslen){
    return platform_socket_recvfrom(sock, buf, size, ss, sslen);
}

ssize_t cdk_net_sendto2(cdk_sock_t sock, void* buf, int size, struct sockaddr_storage* ss, socklen_t sslen){
    return platform_socket_sendto(sock, buf, size, ss, sslen);
}

void cdk_net_close2(cdk_sock_t sock){
    platform_socket_close(sock);
}

void cdk_net_recvtimeo2(cdk_sock_t sock, int timeout_ms) {
    platform_socket_recvtimeo(sock, timeout_ms);
}

void cdk_net_sendtimeo2(cdk_sock_t sock, int timeout_ms) {
    platform_socket_sendtimeo(sock, timeout_ms);
}