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
#include "cdk-channel.h"
#include "cdk-poller.h"
#include "cdk/cdk-time.h"
#include "cdk/cdk-timer.h"
#include "cdk/cdk-utils.h"

cdk_timer_t timer;
bool secure;
static cdk_list_t pollerlst;
static cdk_poller_t* mainpoller;
static mtx_t pollermtx;

typedef struct event_ctx_s {
    cdk_channel_t* channel;
    cdk_txlist_node_t* node;
}event_ctx_t;

static void __channelsend_callback(void* param) {
    event_ctx_t* pctx = param;
    event_ctx_t ctx = *pctx;

    free(pctx);
    pctx = NULL;

    if (!ctx.channel) {
        return;
    }
    if (!ctx.channel->active) {
        free(ctx.node);
        ctx.node = NULL;
        return;
    }
    if (ctx.channel->type == SOCK_STREAM) {
        cdk_list_insert_tail(&(ctx.channel->tcp.txlist), &(ctx.node->n));
    }
    if (ctx.channel->type == SOCK_DGRAM) {
        cdk_list_insert_tail(&(ctx.channel->udp.txlist), &(ctx.node->n));
    }
    ctx.channel->cmd = PLATFORM_EVENT_W;
    cdk_channel_modify(ctx.channel);
}

static void __inet_ntop(int af, const void* restrict src, char* restrict dst) {
    if (af == AF_INET) {
        inet_ntop(af, src, dst, INET_ADDRSTRLEN);
    }
    if (af == AF_INET6) {
        inet_ntop(af, src, dst, INET6_ADDRSTRLEN);
    }
}

static void __connect_timeout(void* param) {
    cdk_channel_t* channel = param;
    channel->handler->on_connect_timeout(channel);
}

static void __connect_timeout_callback(void* param) {
    cdk_channel_t* channel = param;

    cdk_event_t* e = malloc(sizeof(cdk_event_t));
    if (e) {
        e->cb = __connect_timeout;
        e->arg = channel;

        cdk_net_postevent(channel->poller, e);
    }
}

cdk_poller_t* _poller_roundrobin(void)
{
    mtx_lock(&pollermtx);
    if (cdk_list_empty(&pollerlst)) {
        return mainpoller;
    }
    static cdk_poller_t* currpoller;
    if (currpoller == NULL) {
        currpoller = cdk_list_data(cdk_list_head(&pollerlst), cdk_poller_t, node);
    }
    else {
        currpoller = cdk_list_data(cdk_list_next(&currpoller->node), cdk_poller_t, node);
    }
    mtx_unlock(&pollermtx);
    return currpoller;
}

void _eventfd_read(cdk_channel_t* channel, void* buf, size_t len)
{
    mtx_lock(&channel->poller->evmtx);
    if (!cdk_list_empty(&channel->poller->evlist))
    {
        cdk_event_t* e = cdk_list_data(cdk_list_head(&channel->poller->evlist), cdk_event_t, node);
        cdk_list_remove(&e->node);
        e->cb(e->arg);

        free(e);
        e = NULL;
    }
    mtx_unlock(&channel->poller->evmtx);
}

void _eventfd_close(cdk_channel_t* channel, char* error) {
    cdk_channel_destroy(channel);
}

static int __workerthread(void* param) {
    cdk_poller_t* poller = platform_poller_create();
    if (poller == NULL) {
        abort();
    }
    mtx_lock(&pollermtx);
    cdk_list_insert_tail(&pollerlst, &poller->node);
    mtx_unlock(&pollermtx);

    platform_poller_poll(poller);
    mtx_lock(&pollermtx);
    cdk_list_remove(&poller->node);
    mtx_unlock(&pollermtx);
    platform_poller_destroy(poller);
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

cdk_channel_t* cdk_net_listen(const char* type, const char* host, const char* port, cdk_handler_t* handler)
{
    cdk_sock_t sock;
    cdk_channel_t* channel;

    if (!strncmp(type, "tcp", strlen("tcp")))
    {
        sock = platform_socket_listen(host, port, SOCK_STREAM);
        channel = cdk_channel_create(mainpoller, sock, PLATFORM_EVENT_A, handler);
    }
    if (!strncmp(type, "udp", strlen("udp")))
    {
        sock = platform_socket_listen(host, port, SOCK_DGRAM);
        channel = cdk_channel_create(_poller_roundrobin(), sock, PLATFORM_EVENT_R, handler);
    }
    return channel;
}

cdk_channel_t* cdk_net_dial(const char* type, const char* host, const char* port, int timeout, cdk_handler_t* handler)
{
    bool connected;
    cdk_sock_t sock;
    cdk_channel_t* channel;
    cdk_addrinfo_t ai;
    struct sockaddr_storage ss;

    connected = false;

    memset(&ai, 0, sizeof(cdk_addrinfo_t));
    memset(&ss, 0, sizeof(struct sockaddr_storage));

    if (!strncmp(type, "tcp", strlen("tcp"))) {

        sock = platform_socket_dial(host, port, SOCK_STREAM, &connected);
        if (connected) {
            channel = cdk_channel_create(_poller_roundrobin(), sock, PLATFORM_EVENT_W, handler);
            if (channel) {
                channel->handler->on_connect(channel);
            }
        }
        else {
            channel = cdk_channel_create(_poller_roundrobin(), sock, PLATFORM_EVENT_C, handler);
            if (channel) {
                if (channel->tcp.ctimer) {
                    channel->tcp.ctimer->routine = __connect_timeout_callback;
                    channel->tcp.ctimer->arg = channel;
                    channel->tcp.ctimer->birthtime = cdk_time_now();
                    channel->tcp.ctimer->expire = timeout;
                    channel->tcp.ctimer->repeat = false;

                    cdk_timer_add(&timer, channel->tcp.ctimer);
                }
            }
        }
    }
    if (!strncmp(type, "udp", strlen("udp"))) {

        sock = platform_socket_dial(host, port, SOCK_DGRAM, NULL);
        channel = cdk_channel_create(_poller_roundrobin(), sock, PLATFORM_EVENT_W, handler);
        if (channel) {
            memcpy(ai.a, host, strlen(host));
            ai.p = (uint16_t)strtoul(port, NULL, 10);
            ai.f = cdk_net_af(sock);

            cdk_net_pton(&ai, &ss);

            channel->udp.peer.ss = ss;
            channel->udp.peer.sslen = sizeof(struct sockaddr_storage);
        }
    }
    return channel;
}

void cdk_net_poll(void) {
    platform_poller_poll(mainpoller);
    platform_poller_destroy(mainpoller);
}

void cdk_net_channelrecv(cdk_channel_t* channel) {
    channel->cmd = PLATFORM_EVENT_R;
    cdk_channel_modify(channel);
}

void cdk_net_channelsend(cdk_channel_t* channel, void* data, size_t size) {

    mtx_lock(&channel->mtx);
    if (!channel->active) {
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

	if (!thrd_equal(channel->poller->tid, thrd_current())) {

        event_ctx_t* pctx = malloc(sizeof(event_ctx_t));
		if (pctx) {
			pctx->channel = channel;
			pctx->node = node;
		}
		cdk_event_t* e = malloc(sizeof(cdk_event_t));
		if (e) {
			e->cb = __channelsend_callback;
			e->arg = pctx;
			cdk_net_postevent(channel->poller, e);
		}
        mtx_unlock(&channel->mtx);
        return;
	}
    mtx_unlock(&channel->mtx);

	if (channel->type == SOCK_STREAM) {
		cdk_list_insert_tail(&(channel->tcp.txlist), &(node->n));
	}
	if (channel->type == SOCK_DGRAM) {
		cdk_list_insert_tail(&(channel->udp.txlist), &(node->n));
	}
    channel->cmd = PLATFORM_EVENT_W;
    cdk_channel_modify(channel);
}

void cdk_net_channelclose(cdk_channel_t* channel) {
    cdk_channel_destroy(channel);
}

void cdk_net_unpacker_init(cdk_channel_t* channel, cdk_unpack_t* unpacker) {
    memcpy(&channel->tcp.unpacker, unpacker, sizeof(cdk_unpack_t));
}

void cdk_net_postevent(cdk_poller_t* poller, cdk_event_t* event) {
	mtx_lock(&poller->evmtx);
	cdk_list_insert_tail(&poller->evlist, &event->node);
	int hardcode = 1;
	platform_socket_send(poller->evfds[0], &hardcode, sizeof(int));
	mtx_unlock(&poller->evmtx);
}

void cdk_net_startup(int ntimerthrd, int nworkerthrd, bool secure)
{
    platform_socket_startup();

    secure = secure;

    cdk_timer_create(&timer, ntimerthrd);
    cdk_list_init(&pollerlst);
    mtx_init(&pollermtx, mtx_plain);

    mainpoller = platform_poller_create();
    if (mainpoller == NULL) {
        abort();
    }
    for (int i = 0; i < nworkerthrd; i++) {
        thrd_t tid;
        thrd_create(&tid, __workerthread, NULL);
        thrd_detach(tid);
    }
    if (secure) {
        
    }
}

void cdk_net_cleanup(void) {
    platform_socket_cleanup();
    cdk_timer_destroy(&timer);
    mtx_destroy(&pollermtx);
}