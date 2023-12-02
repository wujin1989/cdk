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
#include "cdk/net/cdk-net.h"
#include "channel.h"
#include "poller.h"
#include "tls.h"
#include "cdk/cdk-time.h"
#include "cdk/cdk-timer.h"
#include "cdk/cdk-utils.h"

cdk_timer_t timer;
cdk_tls_ctx_t* tlsctx;

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

static void _channel_send(cdk_channel_t* channel, void* data, size_t size) {
    if (data == NULL || size == 0) {
        return;
    }
    cdk_txlist_node_t* node = malloc(sizeof(cdk_txlist_node_t) + size);
    if (!node) {
        return;
    }
    memset(node, 0, sizeof(cdk_txlist_node_t) + size);
    memcpy(node->buf, data, size);
    node->len = size;
    node->off = 0;

    if (channel->type == SOCK_STREAM) {
        cdk_list_insert_tail(&(channel->tcp.txlist), &(node->n));
    }
    if (channel->type == SOCK_DGRAM) {
        cdk_list_insert_tail(&(channel->udp.txlist), &(node->n));
    }
    if (!(channel->events & EVENT_TYPE_W)) {
        platform_event_add(channel->poller->pfd, channel->fd, EVENT_TYPE_W, channel);
        channel->events |= EVENT_TYPE_W;
    }
}

static void __async_channel_send_callback(void* param) {
    async_channel_send_ctx_t* ctx = param;

    _channel_send(ctx->channel, ctx->data, ctx->size);

    free(ctx);
    ctx = NULL;
}

static void _async_channel_send(cdk_channel_t* channel, void* data, size_t size) {
    async_channel_send_ctx_t* ctx = malloc(sizeof(async_channel_send_ctx_t) + size);
    if (ctx) {
        memset(ctx, 0, sizeof(async_channel_send_ctx_t) + size);
        ctx->channel = channel;
        ctx->size = size;
        memcpy(ctx->data, data, size);
    }
    cdk_event_t* e = malloc(sizeof(cdk_event_t));
    if (e) {
        e->cb = __async_channel_send_callback;
        e->arg = ctx;
        cdk_net_postevent(channel->poller, e, true);
    }
}

static void _channel_destroy(cdk_channel_t* channel) {
    channel_destroy(channel);
}

static void __async_channel_destroy_callback(void* param) {
    cdk_channel_t* channel = param;
    _channel_destroy(channel);
}

static void _async_channel_destroy(cdk_channel_t* channel) {
    cdk_event_t* e = malloc(sizeof(cdk_event_t));
    if (e) {
        e->cb = __async_channel_destroy_callback;
        e->arg = channel;
        cdk_net_postevent(channel->poller, e, true);
    }
}

static void __inet_ntop(int af, const void* restrict src, char* restrict dst) {
    if (af == AF_INET) {
        inet_ntop(af, src, dst, INET_ADDRSTRLEN);
    }
    if (af == AF_INET6) {
        inet_ntop(af, src, dst, INET6_ADDRSTRLEN);
    }
}

cdk_poller_t* _poller_roundrobin(void)
{
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

static int __workerthread(void* param) {
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

        __inet_ntop(AF_INET, &si->sin_addr, d);
        ai->p = ntohs(si->sin_port);
        ai->f = AF_INET;
        break;
    }
    case AF_INET6:
    {
        struct sockaddr_in6* si6 = (struct sockaddr_in6*)ss;

        __inet_ntop(AF_INET6, &si6->sin6_addr, d);
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

void cdk_net_listen(const char* type, const char* host, const char* port, cdk_handler_t* handler)
{
    if (!strncmp(type, "tcp", strlen("tcp")))
    {
        cdk_sock_t sock = platform_socket_listen(host, port, SOCK_STREAM);
        cdk_channel_t* channel = channel_create(_poller_roundrobin(), sock, handler);

        platform_event_add(channel->poller->pfd, channel->fd, EVENT_TYPE_A, channel);
        channel->events |= EVENT_TYPE_A;
    }
    if (!strncmp(type, "udp", strlen("udp")))
    {
        cdk_sock_t sock = platform_socket_listen(host, port, SOCK_DGRAM);
        cdk_channel_t* channel = channel_create(_poller_roundrobin(), sock, handler);
        if (channel) {
            channel->handler->on_ready(channel);

            platform_event_add(channel->poller->pfd, channel->fd, EVENT_TYPE_R, channel);
            channel->events |= EVENT_TYPE_R;
        }
    }
}

void cdk_net_dial(const char* type, const char* host, const char* port, cdk_handler_t* handler)
{
    cdk_addrinfo_t ai;
    struct sockaddr_storage ss;

    memset(&ai, 0, sizeof(cdk_addrinfo_t));
    memset(&ss, 0, sizeof(struct sockaddr_storage));

    if (!strncmp(type, "tcp", strlen("tcp"))) {
        bool connected;
        cdk_sock_t sock = platform_socket_dial(host, port, SOCK_STREAM, &connected);
        cdk_channel_t* channel = channel_create(_poller_roundrobin(), sock, handler);
        if (channel) {
            if (connected) {
                if (channel->tcp.tls) {
                    tls_cli_handshake(channel);
                }
                else {
                    channel->handler->on_connect(channel);

                    platform_event_add(channel->poller->pfd, channel->fd, EVENT_TYPE_R, channel);
                    channel->events |= EVENT_TYPE_R;
                }
                return;
            }
            platform_event_add(channel->poller->pfd, channel->fd, EVENT_TYPE_C, channel);
            channel->events |= EVENT_TYPE_C;
        }
    }
    if (!strncmp(type, "udp", strlen("udp"))) {
        cdk_sock_t sock = platform_socket_dial(host, port, SOCK_DGRAM, NULL);
        cdk_channel_t* channel = channel_create(_poller_roundrobin(), sock, handler);
        if (channel) {
            memcpy(ai.a, host, strlen(host));
            ai.p = (uint16_t)strtoul(port, NULL, 10);
            ai.f = cdk_net_af(sock);

            cdk_net_pton(&ai, &ss);

            channel->udp.peer.ss = ss;
	    /**
	     * In MacOS, when the destination address parameter of the sendto function is of type struct sockaddr_storage, 
	     * the address length cannot use sizeof(struct sockaddr_storage). This seems to be a bug in MacOS.
	     */
            channel->udp.peer.sslen = (ai.f == AF_INET) ? sizeof(struct sockaddr_in): sizeof(struct sockaddr_in6);

            channel->handler->on_ready(channel);

            platform_event_add(channel->poller->pfd, channel->fd, EVENT_TYPE_R, channel);
            channel->events |= EVENT_TYPE_R;
        }
    }
}

bool cdk_net_send(cdk_channel_t* channel, void* data, size_t size) {
    if (!atomic_load(&channel->active)) {
        return false;
    }
	if (thrd_equal(channel->poller->tid, thrd_current())) {
        _channel_send(channel, data, size);
    }
    else {
        _async_channel_send(channel, data, size);
    }
    return true;
}

void cdk_net_close(cdk_channel_t* channel) {
    if (thrd_equal(channel->poller->tid, thrd_current())) {
        _channel_destroy(channel);
    }
    else {
        _async_channel_destroy(channel);
    }
}

void cdk_net_unpacker_init(cdk_channel_t* channel, cdk_unpack_t* unpacker) {
    memcpy(&channel->tcp.unpacker, unpacker, sizeof(cdk_unpack_t));
}

void cdk_net_postevent(cdk_poller_t* poller, cdk_event_t* event, bool totail) {
	mtx_lock(&poller->evmtx);
    if (totail) {
        cdk_list_insert_tail(&poller->evlist, &event->node);
    }
    else {
        cdk_list_insert_head(&poller->evlist, &event->node);
    }
	int hardcode = 1;
	platform_socket_send(poller->evfds[0], &hardcode, sizeof(int));
	mtx_unlock(&poller->evmtx);
}

void cdk_net_startup(int nworkers, cdk_tlsconf_t* tlsconf)
{
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
        thrd_create(workers + i, __workerthread, NULL);
    }
    if (tlsconf) {
        tlsctx = tls_ctx_create(tlsconf);
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
    tls_ctx_destroy(tlsctx);
}
