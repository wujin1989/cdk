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
#include "platform/platform-poller.h"
#include "platform/platform-connection.h"
#include "cdk/container/cdk-list.h"
#include "cdk/net/cdk-net.h"
#include "cdk-connection.h"
#include "cdk/cdk-timer.h"
#include "cdk/cdk-utils.h"

cdk_timer_t timer;
static atomic_flag timer_once_create = ATOMIC_FLAG_INIT;
static atomic_flag timer_once_destroy = ATOMIC_FLAG_INIT;
static atomic_flag one_master_poll = ATOMIC_FLAG_INIT;

typedef struct ctx_s {
    cdk_net_conn_t* conn;
    cdk_txlist_node_t* node;
}ctx_t;

static void __timer_create(void) {
    if (atomic_flag_test_and_set(&timer_once_create)) {
        return;
    }
    cdk_timer_create(&timer, 1);
}

static void __timer_destroy(void) {
    if (atomic_flag_test_and_set(&timer_once_destroy)) {
        return;
    }
    cdk_timer_destroy(&timer);
}

static void __postsend(void* param) {
    ctx_t* pctx = param;
    ctx_t ctx = *pctx;

    free(pctx);
    pctx = NULL;

    if (!ctx.conn) {
        return;
    }
    if (!ctx.conn->active) {
        free(ctx.node);
        ctx.node = NULL;
        return;
    }
    if (ctx.conn->type == SOCK_STREAM) {
        cdk_list_insert_tail(&(ctx.conn->tcp.txlist), &(ctx.node->n));
    }
    if (ctx.conn->type == SOCK_DGRAM) {
        cdk_list_insert_tail(&(ctx.conn->udp.txlist), &(ctx.node->n));
    }
    ctx.conn->cmd = PLATFORM_EVENT_W;
    cdk_connection_modify(ctx.conn);
}

static void __inet_ntop(int af, const void* restrict src, char* restrict dst) {
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

cdk_net_conn_t* cdk_net_listen(const char* type, const char* host, const char* port, cdk_net_handler_t* handler)
{
    cdk_sock_t sock;
    cdk_net_conn_t* conn;

    platform_socket_startup();
    platform_poller_create();
    __timer_create();

    if (!strncmp(type, "tcp", strlen("tcp")))
    {
        sock = platform_socket_listen(host, port, SOCK_STREAM);
        conn = cdk_connection_create(platform_poller_retrieve(true), sock, PLATFORM_EVENT_A, handler);
    }
    if (!strncmp(type, "udp", strlen("udp")))
    {
        sock = platform_socket_listen(host, port, SOCK_DGRAM);
        conn = cdk_connection_create(platform_poller_retrieve(false), sock, PLATFORM_EVENT_R, handler);
    }
    return conn;
}

cdk_net_conn_t* cdk_net_dial(const char* type, const char* host, const char* port, int timeout, cdk_net_handler_t* handler)
{
    bool connected;
    cdk_sock_t sock;
    cdk_net_conn_t* conn;
    cdk_addrinfo_t ai;
    struct sockaddr_storage ss;

    connected = false;

    platform_socket_startup();
    platform_poller_create();
    __timer_create();
    
    memset(&ai, 0, sizeof(cdk_addrinfo_t));
    memset(&ss, 0, sizeof(struct sockaddr_storage));

    if (!strncmp(type, "tcp", strlen("tcp"))) {

        sock = platform_socket_dial(host, port, SOCK_STREAM, &connected);
        if (connected) {
            conn = cdk_connection_create(platform_poller_retrieve(false), sock, PLATFORM_EVENT_W, handler);
            if (!conn) {
                return NULL;
            }
            conn->tcp.connected = true;
            conn->h->on_connect(conn);
        }
        else {
            conn = cdk_connection_create(platform_poller_retrieve(false), sock, PLATFORM_EVENT_C, handler);
            if (!conn) {
                return NULL;
            }
            cdk_timer_add(&timer, platform_connection_connect_timeout, conn, timeout, false);
        }
    }
    if (!strncmp(type, "udp", strlen("udp"))) {

        sock = platform_socket_dial(host, port, SOCK_DGRAM, NULL);
        conn = cdk_connection_create(platform_poller_retrieve(false), sock, PLATFORM_EVENT_W, handler);
        if (!conn) {
            return NULL;
        }
        memcpy(ai.a, host, strlen(host));
        ai.p = (uint16_t)strtoul(port, NULL, 10);
        ai.f = cdk_net_af(sock);

        cdk_net_pton(&ai, &ss);

        conn->udp.peer.ss = ss;
        conn->udp.peer.sslen = sizeof(struct sockaddr_storage);
    }
    return conn;
}

void cdk_net_poll(void) {

    if (atomic_flag_test_and_set(&one_master_poll)) {
        return;
    }
    platform_poller_poll(platform_poller_retrieve(true));
    __timer_destroy();
    platform_poller_destroy();
    platform_socket_cleanup();
}

void cdk_net_postrecv(cdk_net_conn_t* conn) {
    conn->cmd = PLATFORM_EVENT_R;
    cdk_connection_modify(conn);
}

void cdk_net_postsend(cdk_net_conn_t* conn, void* data, size_t size) {

    mtx_lock(&conn->mtx);
    if (!conn->active) {
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

	if (conn->poller->tid != cdk_utils_systemtid()) {

		ctx_t* pctx = malloc(sizeof(ctx_t));
		if (pctx) {
			pctx->conn = conn;
			pctx->node = node;
		}
		cdk_event_t* e = malloc(sizeof(cdk_event_t));
		if (e) {
			e->cb = __postsend;
			e->arg = pctx;
			cdk_net_postevent(conn->poller, e);
		}
        mtx_unlock(&conn->mtx);
        return;
	}
    mtx_unlock(&conn->mtx);

	if (conn->type == SOCK_STREAM) {
		cdk_list_insert_tail(&(conn->tcp.txlist), &(node->n));
	}
	if (conn->type == SOCK_DGRAM) {
		cdk_list_insert_tail(&(conn->udp.txlist), &(node->n));
	}
    conn->cmd = PLATFORM_EVENT_W;
    cdk_connection_modify(conn);
}

void cdk_net_close(cdk_net_conn_t* conn) {
    cdk_connection_destroy(conn);
}

void cdk_net_setup_unpacker(cdk_net_conn_t* conn, cdk_unpack_t* unpacker) {
    memcpy(&conn->tcp.unpacker, unpacker, sizeof(cdk_unpack_t));
}

void cdk_net_concurrent_slaves(int num) {
    platform_poller_concurrent_slaves(num);
}

void cdk_net_postevent(cdk_poller_t* poller, cdk_event_t* event) {
    mtx_lock(&poller->evmtx);
    cdk_list_insert_tail(&poller->evlist, &event->node);
    int hardcode = 1;
    platform_socket_send(poller->evfds[0], &hardcode, sizeof(int));
    mtx_unlock(&poller->evmtx);
}