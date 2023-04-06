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
#include "cdk/container/cdk-rbtree.h"
#include "cdk/container/cdk-list.h"
#include "cdk-net-connection.h"
#include "cdk-net-unpack.h"

#define MAX_IOBUF_SIZE 4096

void cdk_net_connection_postaccept(cdk_net_conn_t* conn) {
    conn->cmd = PLATFORM_EVENT_A;
    cdk_net_connection_modify(conn);
}

void cdk_net_connection_postrecv(cdk_net_conn_t* conn) {
    conn->cmd = PLATFORM_EVENT_R;
    cdk_net_connection_modify(conn);
}

void cdk_net_connection_postsend(cdk_net_conn_t* conn) {
    conn->cmd = PLATFORM_EVENT_W;
    cdk_net_connection_modify(conn);
}

static void __connection_handle_accept(cdk_net_conn_t* conn) {
    cdk_sock_t cli = platform_socket_accept(conn->fd);
    if (cli == -1) {
        if ((errno != EAGAIN || errno != EWOULDBLOCK)) {
            conn->h->on_close(conn, errno);
        }
        if ((errno == EAGAIN || errno == EWOULDBLOCK)) {
            cdk_net_connection_postaccept(conn);
        }
        return;
    }
    cdk_net_conn_t* nconn = cdk_net_connection_create(platform_poller_retrieve(false), cli, PLATFORM_EVENT_R, conn->h);
    conn->h->on_accept(nconn);
    return;
}

static void __connection_handle_recv(cdk_net_conn_t* conn) {
    ssize_t n;

    if (conn->type == SOCK_STREAM) {
        n = platform_socket_recv(conn->fd, (char*)(conn->tcp.rxbuf.buf) + conn->tcp.rxbuf.off, MAX_IOBUF_SIZE);

        if (n == -1) {
            if ((errno != EAGAIN || errno != EWOULDBLOCK)) {
                conn->h->on_close(conn, errno);
            }
            if ((errno == EAGAIN || errno == EWOULDBLOCK)) {
                cdk_net_connection_postrecv(conn);
            }
            return;
        }
        if (n == 0) {
            conn->h->on_close(conn, errno);
            return;
        }
        conn->tcp.rxbuf.off += n;
        cdk_net_unpack(conn);
    }
    if (conn->type == SOCK_DGRAM) {

        conn->udp.peer.sslen = sizeof(struct sockaddr_storage);
        n = platform_socket_recvfrom(conn->fd, conn->udp.rxbuf.buf, MAX_IOBUF_SIZE, &conn->udp.peer.ss, &conn->udp.peer.sslen);

        if (n == -1) {
            if ((errno != EAGAIN || errno != EWOULDBLOCK)) {
                conn->h->on_close(conn, errno);
            }
            if ((errno == EAGAIN || errno == EWOULDBLOCK)) {
                cdk_net_connection_postrecv(conn);
            }
            return;
        }
        conn->h->on_read(conn, conn->udp.rxbuf.buf, n);
    }
    return;
}

static void __connection_handle_connect(cdk_net_conn_t* conn) {
    int err;
    socklen_t len;
    len = sizeof(int);
    getsockopt(conn->fd, SOL_SOCKET, SO_ERROR, (char*)&err, &len);
    if (err) {
        conn->h->on_close(conn, errno);
    }
    else {
        conn->h->on_connect(conn);
    }
}

static void __connection_handle_send(cdk_net_conn_t* conn) {
    if (conn->type == SOCK_STREAM) {
        mtx_lock(&conn->txmtx);

        while (cdk_list_empty(&(conn->tcp.txlist))) {
            mtx_unlock(&conn->txmtx);
            return;
        }
        while (!cdk_list_empty(&(conn->tcp.txlist))) {

            inner_offset_buf_t* e = cdk_list_data(cdk_list_head(&(conn->tcp.txlist)), inner_offset_buf_t, n);

            while (e->off < e->len) {

                ssize_t n = platform_socket_send(conn->fd, e->buf + e->off, (int)(e->len - e->off));
                if (n == -1) {
                    if ((errno != EAGAIN || errno != EWOULDBLOCK)) {
                        conn->h->on_close(conn, errno);
                    }
                    if ((errno == EAGAIN || errno == EWOULDBLOCK)) {
                        cdk_net_connection_postsend(conn);
                        mtx_unlock(&conn->txmtx);
                    }
                    return;
                }
                if (n == 0) {
                    conn->h->on_close(conn, errno);
                    return;
                }
                e->off += n;
            }
            conn->h->on_write(conn, e->buf, e->len);
            cdk_list_remove(&(e->n));
            free(e);
            e = NULL;
        }
        mtx_unlock(&conn->txmtx);
    }
    if (conn->type == SOCK_DGRAM) {
        mtx_lock(&conn->txmtx);

        while (cdk_list_empty(&(conn->udp.txlist))) {
            mtx_unlock(&conn->txmtx);
            return;
        }
        while (!cdk_list_empty(&(conn->udp.txlist))) {

            inner_offset_buf_t* e = cdk_list_data(cdk_list_head(&(conn->udp.txlist)), inner_offset_buf_t, n);

            ssize_t n = platform_socket_sendto(conn->fd, e->buf, (int)e->len, &(conn->udp.peer.ss), conn->udp.peer.sslen);
            if (n == -1) {
                if ((errno != EAGAIN || errno != EWOULDBLOCK)) {
                    conn->h->on_close(conn, errno);
                }
                if ((errno == EAGAIN || errno == EWOULDBLOCK)) {
                    cdk_net_connection_postsend(conn);
                    mtx_unlock(&conn->txmtx);
                }
                return;
            }
            if (n == 0) {
                conn->h->on_close(conn, errno);
                return;
            }
            conn->h->on_write(conn, e->buf, e->len);

            cdk_list_remove(&(e->n));
            free(e);
            e = NULL;
        }
        mtx_unlock(&conn->txmtx);
    }
    return;
}

cdk_net_conn_t* cdk_net_connection_create(cdk_poller_t* poller, cdk_sock_t sock, int cmd, cdk_net_handler_t* handler)
{
    cdk_net_conn_t* conn = malloc(sizeof(cdk_net_conn_t));

    if (conn) {
        conn->poller = *poller;
        conn->cmd = cmd;
        conn->fd = sock;
        conn->h = handler;
        conn->type = platform_socket_socktype(sock);
        conn->active = true;
        mtx_init(&conn->txmtx, mtx_plain);

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

void cdk_net_connection_modify(cdk_net_conn_t* conn) {
    platform_event_mod(conn->poller.pfd, conn->fd, conn->cmd, conn);
}

void cdk_net_connection_destroy(cdk_net_conn_t* conn) {
    platform_event_del(conn->poller.pfd, conn->fd);
    platform_socket_close(conn->fd);

    mtx_lock(&conn->txmtx);
    if (conn->type == SOCK_STREAM) {
        free(conn->tcp.rxbuf.buf);
        conn->tcp.rxbuf.buf = NULL;

        while (!cdk_list_empty(&(conn->tcp.txlist))) {
            inner_offset_buf_t* e = cdk_list_data(cdk_list_head(&(conn->tcp.txlist)), inner_offset_buf_t, n);
            cdk_list_remove(&(e->n));
            free(e);
            e = NULL;
        }
    }
    if (conn->type == SOCK_DGRAM) {
        free(conn->udp.rxbuf.buf);
        conn->udp.rxbuf.buf = NULL;

        while (!cdk_list_empty(&(conn->udp.txlist))) {
            inner_offset_buf_t* e = cdk_list_data(cdk_list_head(&(conn->udp.txlist)), inner_offset_buf_t, n);
            cdk_list_remove(&(e->n));
            free(e);
            e = NULL;
        }
    }
    mtx_unlock(&conn->txmtx);
    return;
}

void cdk_net_connection_process(cdk_net_conn_t* conn)
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