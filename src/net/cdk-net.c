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

#include "cdk/net/cdk-net.h"
#include "cdk/cdk-logger.h"
#include "cdk/cdk-time.h"
#include "cdk/cdk-timer.h"
#include "cdk/cdk-utils.h"
#include "cdk/container/cdk-list.h"
#include "cdk/sync/cdk-waitgroup.h"
#include "channel.h"
#include "platform/platform-event.h"
#include "platform/platform-socket.h"
#include "poller.h"
#include "tls.h"
#include "txlist.h"

cdk_net_engine_t global_net_engine = {.initialized = ATOMIC_FLAG_INIT};

typedef struct channel_send_ctx_s {
    cdk_channel_t* channel;
    size_t         size;
    char           data[];
} channel_send_ctx_t;

typedef struct socket_ctx_s {
    char           host[INET6_ADDRSTRLEN];
    char           port[6];
    int            protocol;
    int            idx;
    int            cores;
    cdk_poller_t*  poller;
    cdk_handler_t* handler;
    cdk_tls_ctx_t* tls_ctx;
} socket_ctx_t;

static void _async_poller_exit(void* param) {
    cdk_poller_t* poller = param;
    poller->active = false;
}

static inline cdk_poller_t* _roundrobin(void) {
    mtx_lock(&global_net_engine.poller_mtx);
    while (cdk_list_empty(&global_net_engine.poller_lst)) {
        cnd_wait(&global_net_engine.poller_cnd, &global_net_engine.poller_mtx);
    }
    static cdk_poller_t* curr;
    if (curr == NULL) {
        curr = cdk_list_data(
            cdk_list_head(&global_net_engine.poller_lst), cdk_poller_t, node);
    } else {
        cdk_list_node_t* n = cdk_list_next(&curr->node);
        if (n != cdk_list_sentinel(&global_net_engine.poller_lst)) {
            curr = cdk_list_data(n, cdk_poller_t, node);
        } else {
            curr = cdk_list_data(cdk_list_next(n), cdk_poller_t, node);
        }
    }
    mtx_unlock(&global_net_engine.poller_mtx);
    return curr;
}

static void _net_engine_add_poller(cdk_poller_t* poller) {
    mtx_lock(&global_net_engine.poller_mtx);
    cdk_list_insert_tail(&global_net_engine.poller_lst, &poller->node);
    cnd_signal(&global_net_engine.poller_cnd);
    mtx_unlock(&global_net_engine.poller_mtx);
}

static void _net_engine_del_poller(cdk_poller_t* poller) {
    mtx_lock(&global_net_engine.poller_mtx);
    cdk_list_remove(&poller->node);
    mtx_unlock(&global_net_engine.poller_mtx);
}

static void _net_engine_destroy(void) {
    free(global_net_engine.thrdids);
    global_net_engine.thrdids = NULL;

    mtx_destroy(&global_net_engine.poller_mtx);
    cnd_destroy(&global_net_engine.poller_cnd);
    cdk_waitgroup_destroy(global_net_engine.wg);
    platform_socket_cleanup();
}

static int _poll(void* param) {
    cdk_poller_t* poller = poller_create();
    if (!poller) {
        return -1;
    }
    _net_engine_add_poller(poller);
    poller_poll(poller);
    _net_engine_del_poller(poller);
    poller_destroy(poller);

    cdk_waitgroup_done(global_net_engine.wg);
    atomic_fetch_sub(&global_net_engine.thrdcnt, 1);
    return 0;
}

static void _net_engine_create(void) {
    platform_socket_startup();
    if (!atomic_load(&global_net_engine.thrdcnt)) {
        atomic_store(&global_net_engine.thrdcnt, 1);
    }
    cdk_list_init(&global_net_engine.poller_lst);
    mtx_init(&global_net_engine.poller_mtx, mtx_plain);
    cnd_init(&global_net_engine.poller_cnd);

    global_net_engine.wg = cdk_waitgroup_create();
    cdk_waitgroup_add(
        global_net_engine.wg, atomic_load(&global_net_engine.thrdcnt));

    global_net_engine.poller_roundrobin = _roundrobin;
    global_net_engine.thrdids =
        malloc(atomic_load(&global_net_engine.thrdcnt) * sizeof(thrd_t));
    if (!global_net_engine.thrdids) {
        return;
    }
    for (int i = 0; i < atomic_load(&global_net_engine.thrdcnt); i++) {
        thrd_create((global_net_engine.thrdids + i), _poll, NULL);
        thrd_detach(global_net_engine.thrdids[i]);
    }
}

static socket_ctx_t* _socket_ctx_allocate(
    const char*    protocol,
    const char*    host,
    const char*    port,
    int            idx,
    int            cores,
    cdk_handler_t* handler,
    cdk_tls_ctx_t* tlsctx) {
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
        ctx->tls_ctx = tlsctx;
        ctx->poller = global_net_engine.poller_roundrobin();
    }
    return ctx;
}

static void _async_listen(void* param) {
    socket_ctx_t* sctx = param;

    cdk_sock_t sock = platform_socket_listen(
        sctx->host, sctx->port, sctx->protocol, sctx->idx, sctx->cores, true);

    cdk_channel_t* channel = channel_create(
        sctx->poller,
        sock,
        CHANNEL_MODE_ACCEPT,
        SIDE_SERVER,
        sctx->handler,
        sctx->tls_ctx);
    if (!channel) {
        free(sctx);
        sctx = NULL;
        return;
    }
    channel->accepting = true;

    if (!channel_is_reading(channel)) {
        channel_enable_read(channel);
    }
    free(sctx);
    sctx = NULL;
}

static inline void _conn_timeout_cb(void* param) {
    cdk_channel_t* channel = param;

    cdk_channel_error_t error = {
        .code = CHANNEL_ERROR_CONN_TIMEOUT,
        .codestr = CHANNEL_ERROR_CONN_TIMEOUT_STR};
    channel_error_update(channel, error);
    channel_destroy(channel);
}

static void _async_dial(void* param) {
    socket_ctx_t* sctx = param;
    bool          connected = false;

    cdk_sock_t sock = platform_socket_dial(
        sctx->host, sctx->port, sctx->protocol, &connected, true);

    cdk_channel_t* channel = channel_create(
        sctx->poller,
        sock,
        CHANNEL_MODE_CONNECT,
        SIDE_CLIENT,
        sctx->handler,
        sctx->tls_ctx);

    if (!channel) {
        free(sctx);
        sctx = NULL;
        return;
    }
    if (channel->type == SOCK_STREAM) {
        if (connected) {
            if (channel->tcp.tls_ssl) {
                channel_tls_cli_handshake(channel);
            } else {
                channel_connected(channel);
            }
        } else {
            channel->tcp.connecting = true;
            if (!channel_is_writing(channel)) {
                channel_enable_write(channel);
            }
            if (channel->handler->conn_timeout) {
                channel->tcp.conn_timer = cdk_timer_add(
                    channel->poller->timermgr,
                    _conn_timeout_cb,
                    channel,
                    channel->handler->conn_timeout,
                    false);
            }
        }
    } else {
        cdk_net_address_make(
            sock, &channel->udp.peer.ss, sctx->host, sctx->port);
        /**
         * In MacOS, when the destination address parameter of the sendto
         * function is of type struct sockaddr_storage, the address length
         * cannot use sizeof(struct sockaddr_storage). This seems to be a
         * bug in MacOS.
         */
        channel->udp.peer.sslen =
            (platform_socket_extract_family(sock) == AF_INET)
                ? sizeof(struct sockaddr_in)
                : sizeof(struct sockaddr_in6);

        channel_connected(channel);
    }
    free(sctx);
    sctx = NULL;
}

static void _async_channel_explicit_send(void* param) {
    channel_send_ctx_t* ctx = param;

    channel_explicit_send(ctx->channel, ctx->data, ctx->size);
    free(ctx);
    ctx = NULL;
}

static void _async_channel_destroy(void* param) {
    cdk_channel_t* channel = param;

    cdk_channel_error_t error = {
        .code = CHANNEL_ERROR_USER_CLOSE,
        .codestr = CHANNEL_ERROR_USER_CLOSE_STR};
    channel_error_update(channel, error);

    channel_destroy(channel);
}

static void _inet_ntop(int af, const void* restrict src, char* restrict dst) {
    switch (af) {
    case AF_INET: {
        inet_ntop(af, src, dst, INET_ADDRSTRLEN);
        break;
    }
    case AF_INET6: {
        inet_ntop(af, src, dst, INET6_ADDRSTRLEN);
        break;
    }
    default:
        break;
    }
}

void cdk_net_ntop(struct sockaddr_storage* ss, cdk_address_t* ai) {
    char addr[INET6_ADDRSTRLEN];
    memset(addr, 0, sizeof(addr));

    switch (ss->ss_family) {
    case AF_INET: {
        struct sockaddr_in* si = (struct sockaddr_in*)ss;

        _inet_ntop(AF_INET, &si->sin_addr, addr);
        ai->port = ntohs(si->sin_port);
        ai->family = AF_INET;
        break;
    }
    case AF_INET6: {
        struct sockaddr_in6* si6 = (struct sockaddr_in6*)ss;

        _inet_ntop(AF_INET6, &si6->sin6_addr, addr);
        ai->port = ntohs(si6->sin6_port);
        ai->family = AF_INET6;
        break;
    }
    default:
        return;
    }
    memcpy(ai->addr, addr, INET6_ADDRSTRLEN);
}

void cdk_net_pton(cdk_address_t* ai, struct sockaddr_storage* ss) {
    memset(ss, 0, sizeof(struct sockaddr_storage));

    switch (ai->family) {
    case AF_INET: {
        struct sockaddr_in* si = (struct sockaddr_in*)ss;

        si->sin_family = AF_INET;
        si->sin_port = htons(ai->port);
        inet_pton(AF_INET, ai->addr, &(si->sin_addr));
        break;
    }
    case AF_INET6: {
        struct sockaddr_in6* si6 = (struct sockaddr_in6*)ss;

        si6->sin6_family = AF_INET6;
        si6->sin6_port = htons(ai->port);
        inet_pton(AF_INET6, ai->addr, &(si6->sin6_addr));
        break;
    }
    default:
        break;
    }
}

void cdk_net_address_make(
    cdk_sock_t sock, struct sockaddr_storage* ss, char* host, char* port) {
    cdk_address_t ai = {0};

    memcpy(ai.addr, host, strlen(host));
    ai.port = (uint16_t)strtoul(port, NULL, 10);
    ai.family = platform_socket_extract_family(sock);

    cdk_net_pton(&ai, ss);
}

void cdk_net_address_retrieve(cdk_sock_t sock, cdk_address_t* ai, bool peer) {
    struct sockaddr_storage ss;
    socklen_t               len;

    len = sizeof(struct sockaddr_storage);

    if (peer) {
        getpeername(sock, (struct sockaddr*)&ss, &len);
    } else {
        getsockname(sock, (struct sockaddr*)&ss, &len);
    }
    cdk_net_ntop(&ss, ai);
}

int cdk_net_getsocktype(cdk_sock_t sock) {
    return platform_socket_getsocktype(sock);
}

void cdk_net_concurrency_configure(int ncpus) {
    atomic_init(&global_net_engine.thrdcnt, ncpus);
}

void cdk_net_listen(
    const char*    protocol,
    const char*    host,
    const char*    port,
    cdk_handler_t* handler) {

    if (!atomic_flag_test_and_set(&global_net_engine.initialized)) {
        _net_engine_create();
    }
    /**
     * Destroy tlsctx when the accepting channel is destroyed.
     */
    cdk_tls_ctx_t* tlsctx = tls_ctx_create(handler->tlsconfig);

    for (int i = 0; i < atomic_load(&global_net_engine.thrdcnt); i++) {
        socket_ctx_t* sctx = _socket_ctx_allocate(
            protocol,
            host,
            port,
            i,
            atomic_load(&global_net_engine.thrdcnt),
            handler,
            tlsctx);
        if (sctx) {
            cdk_net_post_event(sctx->poller, _async_listen, sctx, true);
        }
    }
}

void cdk_net_dial(
    const char*    protocol,
    const char*    host,
    const char*    port,
    cdk_handler_t* handler) {

    if (!atomic_flag_test_and_set(&global_net_engine.initialized)) {
        _net_engine_create();
    }
    cdk_tls_ctx_t* tlsctx = tls_ctx_create(handler->tlsconfig);

    socket_ctx_t* sctx =
        _socket_ctx_allocate(protocol, host, port, 0, 0, handler, tlsctx);
    if (sctx) {
        cdk_net_post_event(sctx->poller, _async_dial, sctx, true);
    }
}

bool cdk_net_send(cdk_channel_t* channel, void* data, size_t size) {
    if (atomic_load(&channel->closing)) {
        return false;
    }
    if (thrd_equal(channel->poller->tid, thrd_current())) {
        channel_explicit_send(channel, data, size);
    } else {
        channel_send_ctx_t* ctx = malloc(sizeof(channel_send_ctx_t) + size);
        if (!ctx) {
            return false;
        }
        memset(ctx, 0, sizeof(channel_send_ctx_t) + size);
        ctx->channel = channel;
        ctx->size = size;
        memcpy(ctx->data, data, size);

        cdk_net_post_event(
            channel->poller, _async_channel_explicit_send, ctx, true);
    }
    return true;
}

void cdk_net_close(cdk_channel_t* channel) {
    if (atomic_load(&channel->closing)) {
        return;
    }
    if (thrd_equal(channel->poller->tid, thrd_current())) {
        cdk_channel_error_t error = {
            .code = CHANNEL_ERROR_USER_CLOSE,
            .codestr = CHANNEL_ERROR_USER_CLOSE_STR};
        channel_error_update(channel, error);
        channel_destroy(channel);
    } else {
        cdk_net_post_event(
            channel->poller, _async_channel_destroy, channel, true);
    }
}

void cdk_net_post_event(
    cdk_poller_t* poller, void (*task)(void*), void* arg, bool totail) {
    cdk_async_event_t* async_event = malloc(sizeof(cdk_async_event_t));
    if (!async_event) {
        return;
    }
    async_event->task = task;
    async_event->arg = arg;

    mtx_lock(&poller->evmtx);
    if (totail) {
        cdk_list_insert_tail(&poller->evlist, &async_event->node);
    } else {
        cdk_list_insert_head(&poller->evlist, &async_event->node);
    }
    mtx_unlock(&poller->evmtx);
    poller_wakeup(poller);
}

void cdk_net_timer_create(
    void (*routine)(void*), void* param, size_t expire, bool repeat) {
    cdk_poller_t* poller = global_net_engine.poller_roundrobin();
    if (!poller) {
        return;
    }
    if (!poller->active) {
        return;
    }
    cdk_timer_add(poller->timermgr, routine, param, expire, repeat);
}

void cdk_net_exit(void) {
    if (!atomic_flag_test_and_set(&global_net_engine.initialized)) {
        return;
    }
    atomic_flag_clear(&global_net_engine.initialized);

    cdk_list_node_t* node = NULL;

    mtx_lock(&global_net_engine.poller_mtx);
    node = cdk_list_head(&global_net_engine.poller_lst);
    mtx_unlock(&global_net_engine.poller_mtx);

    while (node != cdk_list_sentinel(&global_net_engine.poller_lst)) {
        mtx_lock(&global_net_engine.poller_mtx);
        cdk_poller_t* poller = cdk_list_data(node, cdk_poller_t, node);
        node = cdk_list_next(node);
        mtx_unlock(&global_net_engine.poller_mtx);

        cdk_net_post_event(poller, _async_poller_exit, poller, false);
    }
    cdk_waitgroup_wait(global_net_engine.wg);

    if (!atomic_load(&global_net_engine.thrdcnt)) {
        _net_engine_destroy();
    }
}
