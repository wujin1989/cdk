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
#include "platform/platform-connection.h"
#include "cdk/container/cdk-list.h"
#include "cdk-connection.h"
#include "cdk/cdk-timer.h"

extern cdk_timer_t timer;

static void __connection_handle_accept(cdk_net_conn_t* conn) {
    platform_connection_accept(conn);
}

static void __connection_handle_recv(cdk_net_conn_t* conn) {
    platform_connection_recv(conn);
}

static void __connection_handle_send(cdk_net_conn_t* conn) {
    platform_connection_send(conn);
}

static void __connection_handle_connect(cdk_net_conn_t* conn) {
    platform_connection_connect(conn);
}

static void __connection_destroy(void* param) {
    cdk_net_conn_t* conn = param;

    mtx_destroy(&conn->mtx);
    free(conn);
    conn = NULL;
}

cdk_net_conn_t* cdk_connection_create(cdk_poller_t* poller, cdk_sock_t sock, int cmd, cdk_net_handler_t* handler)
{
    cdk_net_conn_t* conn = malloc(sizeof(cdk_net_conn_t));

    if (conn) {
        conn->poller = *poller;
        conn->cmd = cmd;
        conn->fd = sock;
        conn->h = handler;
        conn->type = platform_socket_socktype(sock);
        conn->active = true;
        conn->connected = false;
        mtx_init(&conn->mtx, mtx_plain);

        if (conn->type == SOCK_STREAM) {
            conn->tcp.rxbuf.len = MAX_IOBUF_SIZE;
            conn->tcp.rxbuf.off = 0;
            conn->tcp.rxbuf.buf = malloc(MAX_IOBUF_SIZE);
            if (conn->tcp.rxbuf.buf) {
                memset(conn->tcp.rxbuf.buf, 0, MAX_IOBUF_SIZE);
            }
            cdk_list_init(&(conn->tcp.txlist));
        }
        if (conn->type == SOCK_DGRAM) {
            conn->udp.rxbuf.len = MAX_IOBUF_SIZE;
            conn->udp.rxbuf.off = 0;
            conn->udp.rxbuf.buf = malloc(MAX_IOBUF_SIZE);
            if (conn->udp.rxbuf.buf) {
                memset(conn->udp.rxbuf.buf, 0, MAX_IOBUF_SIZE);
            }
            cdk_list_init(&(conn->udp.txlist));
        }
        platform_event_add(poller->pfd, conn->fd, cmd, conn);
        return conn;
    }
    return NULL;
}

void cdk_connection_modify(cdk_net_conn_t* conn) {
    platform_event_mod(conn->poller.pfd, conn->fd, conn->cmd, conn);
}

void cdk_connection_postaccept(cdk_net_conn_t* conn) {
    conn->cmd = PLATFORM_EVENT_A;
    cdk_connection_modify(conn);
}

void cdk_connection_postrecv(cdk_net_conn_t* conn) {
    conn->cmd = PLATFORM_EVENT_R;
    cdk_connection_modify(conn);
}

void cdk_connection_postsend(cdk_net_conn_t* conn) {
    conn->cmd = PLATFORM_EVENT_W;
    cdk_connection_modify(conn);
}

void cdk_connection_destroy(cdk_net_conn_t* conn)
{
    mtx_lock(&conn->mtx);

    platform_event_del(conn->poller.pfd, conn->fd);
    platform_socket_close(conn->fd);

    conn->active = false;

    if (conn->type == SOCK_STREAM) {
        free(conn->tcp.rxbuf.buf);
        conn->tcp.rxbuf.buf = NULL;

        while (!cdk_list_empty(&(conn->tcp.txlist))) {
            cdk_txlist_node_t* e = cdk_list_data(cdk_list_head(&(conn->tcp.txlist)), cdk_txlist_node_t, n);
            cdk_list_remove(&(e->n));
            free(e);
            e = NULL;
        }
    }
    if (conn->type == SOCK_DGRAM) {
        free(conn->udp.rxbuf.buf);
        conn->udp.rxbuf.buf = NULL;

        while (!cdk_list_empty(&(conn->udp.txlist))) {
            cdk_txlist_node_t* e = cdk_list_data(cdk_list_head(&(conn->udp.txlist)), cdk_txlist_node_t, n);
            cdk_list_remove(&(e->n));
            free(e);
            e = NULL;
        }
    }
    mtx_unlock(&conn->mtx);

    cdk_timer_add(&timer, __connection_destroy, conn, 5000, false);
}

void cdk_connection_process(cdk_net_conn_t* conn)
{
    switch (conn->cmd)
    {
    case PLATFORM_EVENT_A:
        __connection_handle_accept(conn);
        break;
    case PLATFORM_EVENT_R:
        __connection_handle_recv(conn);
        break;
    case PLATFORM_EVENT_C:
        __connection_handle_connect(conn);
        break;
    case PLATFORM_EVENT_W:
        __connection_handle_send(conn);
        break;
    default:
        break;
    }
}