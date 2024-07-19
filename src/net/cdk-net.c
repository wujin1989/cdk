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
cdk_tls_ctx_t* tls_ctx;

typedef struct channel_send_ctx_s{
    cdk_channel_t* channel;
    size_t size;
    char data[];
} channel_send_ctx_t;

typedef struct socket_ctx_s {
    char host[INET6_ADDRSTRLEN];
    char port[6];
    int protocol;
    int idx;
    int cores;
    cdk_poller_t* poller;
    cdk_handler_t* handler;
} socket_ctx_t;

static void _cb_inactive(void* param) {
    cdk_poller_t* poller = param;
    poller->active = false;
}

static cdk_poller_t* _roundrobin(void) {
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

static void _manager_add_poller(cdk_poller_t* poller) {
    mtx_lock(&manager.poller_mtx);
    cdk_list_insert_tail(&manager.poller_lst, &poller->node);
    cnd_signal(&manager.poller_cnd);
    mtx_unlock(&manager.poller_mtx);
}

static void _manager_del_poller(cdk_poller_t* poller) {
    mtx_lock(&manager.poller_mtx);
    cdk_list_remove(&poller->node);
    mtx_unlock(&manager.poller_mtx);
}

static int _routine(void* param) {
    cdk_poller_t* poller = poller_create();
    if (poller) {
        _manager_add_poller(poller);
        poller_poll(poller);
        _manager_del_poller(poller);
        poller_destroy(poller);
    }
    return 0;
}

static void _manager_create(int parallel) {
    platform_socket_startup();
    if (parallel <= 0) {
        abort();
    }
    cdk_list_init(&manager.poller_lst);
    mtx_init(&manager.poller_mtx, mtx_plain);
    cnd_init(&manager.poller_cnd);

    manager.poller_roundrobin = _roundrobin;
    manager.thrdcnt = parallel;
    manager.thrdids = malloc(manager.thrdcnt * sizeof(thrd_t));
    for (int i = 0; i < manager.thrdcnt; i++) {
        thrd_create((manager.thrdids + i), _routine, NULL);
    }
}

static void _manager_destroy(void) {
    for (int i = 0; i < manager.thrdcnt; i++) {
        thrd_join(manager.thrdids[i], NULL);
    }
    free(manager.thrdids);
    manager.thrdids = NULL;
    
    mtx_destroy(&manager.poller_mtx);
    cnd_destroy(&manager.poller_cnd);
    platform_socket_cleanup();
}

static socket_ctx_t* _socket_ctx_allocate(const char* protocol, const char* host, const char* port, int idx, int cores, cdk_handler_t* handler) {
    socket_ctx_t* ctx = malloc(sizeof(socket_ctx_t));
    if (ctx) {
        memset(ctx, 0, sizeof(socket_ctx_t));
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

static void _cb_listen(void* param) {
    socket_ctx_t* ctx = param;

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

static void _cb_dial(void* param) {
    socket_ctx_t* ctx = param;
    bool connected = false;

    cdk_sock_t sock = platform_socket_dial(ctx->host, ctx->port, ctx->protocol, &connected, true);
    cdk_channel_t* channel = channel_create(ctx->poller, sock, ctx->handler);
    if (channel) {
        if (channel->type == SOCK_STREAM) {
            if (connected) {
                if (channel->tcp.tls_ssl) {
                    channel_tls_cli_handshake(channel);
                } else {
                    channel_connected(channel);
                }
            } else {
                channel_connecting(channel);
            }
        }
        if (channel->type == SOCK_DGRAM) {
            cdk_net_make_address(sock, &channel->udp.peer.ss, ctx->host, ctx->port);
            /**
             * In MacOS, when the destination address parameter of the sendto function is of type struct sockaddr_storage,
             * the address length cannot use sizeof(struct sockaddr_storage). This seems to be a bug in MacOS.
             */
            channel->udp.peer.sslen = 
                (platform_socket_extract_family(sock) == AF_INET) 
                ? sizeof(struct sockaddr_in) 
                : sizeof(struct sockaddr_in6);
            
            if (channel->handler->udp.on_ready) {
                channel->handler->udp.on_ready(channel);
            }
            if (!channel_is_reading(channel)) {
                channel_enable_read(channel);
            }
        }
    }
    free(ctx);
    ctx = NULL;
}

static void _cb_channel_send_explicit(void* param) {
    channel_send_ctx_t* ctx = param;
    channel_send_explicit(ctx->channel, ctx->data, ctx->size);
    free(ctx);
    ctx = NULL;
}

static void _channel_send_explicit(cdk_channel_t* channel, void* data, size_t size) {
    channel_send_ctx_t* ctx = malloc(sizeof(channel_send_ctx_t) + size);
    if (ctx) {
        memset(ctx, 0, sizeof(channel_send_ctx_t) + size);
        ctx->channel = channel;
        ctx->size = size;
        memcpy(ctx->data, data, size);

        cdk_net_postevent(channel->poller, _cb_channel_send_explicit, ctx, true);
    }
}

static void _cb_channel_destroy(void* param) {
    cdk_channel_t* channel = param;
    channel_destroy(channel, "");
}

static void _channel_destroy(cdk_channel_t* channel) {
    cdk_net_postevent(channel->poller, _cb_channel_destroy, channel, true);
}

static void _inet_ntop(int af, const void* restrict src, char* restrict dst) {
    if (af == AF_INET) {
        inet_ntop(af, src, dst, INET_ADDRSTRLEN);
    }
    if (af == AF_INET6) {
        inet_ntop(af, src, dst, INET6_ADDRSTRLEN);
    }
}

void cdk_net_ntop(struct sockaddr_storage* ss, cdk_address_t* ai) {
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

void cdk_net_pton(cdk_address_t* ai, struct sockaddr_storage* ss) {
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

void cdk_net_make_address(cdk_sock_t sock, struct sockaddr_storage* ss, char* host, char* port) {
    cdk_address_t ai = { 0 };

    memcpy(ai.a, host, strlen(host));
    ai.p = (uint16_t)strtoul(port, NULL, 10);
    ai.f = platform_socket_extract_family(sock);

    cdk_net_pton(&ai, ss);
}

void cdk_net_extract_address(cdk_sock_t sock, cdk_address_t* ai, bool peer) {
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
        socket_ctx_t* ctx = _socket_ctx_allocate(protocol, host, port, i, manager.thrdcnt, handler);
        if (ctx) {
            cdk_net_postevent(ctx->poller, _cb_listen, ctx, true);
        }
    }
}

void cdk_net_dial(const char* protocol, const char* host, const char* port, cdk_handler_t* handler) {
    socket_ctx_t* ctx = _socket_ctx_allocate(protocol, host, port, 0, 0, handler);
    if (ctx) {
        cdk_net_postevent(ctx->poller, _cb_dial, ctx, true);
    }
}

void cdk_net_send(cdk_channel_t* channel, void* data, size_t size) {
	if (thrd_equal(channel->poller->tid, thrd_current())) {
        channel_send_explicit(channel, data, size);
    } else {
        _channel_send_explicit(channel, data, size);
    }
}

void cdk_net_close(cdk_channel_t* channel) {
    if (thrd_equal(channel->poller->tid, thrd_current())) {
        channel_destroy(channel, NULL);
    } else {
        _channel_destroy(channel);
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
        poller_wakeup(poller);
    }
}

void cdk_net_exit(void) {
    mtx_lock(&manager.poller_mtx);
    for (cdk_list_node_t* n = cdk_list_head(&manager.poller_lst); n != cdk_list_sentinel(&manager.poller_lst); n = cdk_list_next(n)) {
        cdk_poller_t* poller = cdk_list_data(n, cdk_poller_t, node);
        cdk_net_postevent(poller, _cb_inactive, poller, true);
    }
    mtx_unlock(&manager.poller_mtx);
}

void cdk_net_startup(cdk_conf_t* conf) {
    if (atomic_flag_test_and_set(&manager.initialized)) {
        return;
    }
    tls_ctx = tls_ctx_create(&conf->tls);
    _manager_create(conf->nthrds);
}

void cdk_net_cleanup(void) {
    _manager_destroy();
    tls_ctx_destroy(tls_ctx);
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