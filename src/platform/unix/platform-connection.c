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
#include "platform/platform-utils.h"
#include "platform/platform-poller.h"
#include "platform/platform-event.h"
#include "cdk/container/cdk-list.h"
#include "net/cdk-connection.h"
#include "net/cdk-unpack.h"
#include "cdk/net/cdk-net.h"

void platform_connection_recv(cdk_net_conn_t* conn) {
    ssize_t n;

    if (conn->type == SOCK_STREAM) {
        n = platform_socket_recv(conn->fd, (char*)(conn->tcp.rxbuf.buf) + conn->tcp.rxbuf.off, MAX_IOBUF_SIZE);

        if (n == -1) {
            if ((errno != EAGAIN || errno != EWOULDBLOCK)) {
                conn->h->on_close(conn, strerror(errno));
            }
            return;
        }
        if (n == 0) {
            conn->h->on_close(conn, strerror(ECONNRESET));
            return;
        }
        conn->tcp.rxbuf.off += n;
        cdk_unpack(conn);
    }
    if (conn->type == SOCK_DGRAM) {

        conn->udp.peer.sslen = sizeof(struct sockaddr_storage);
        n = platform_socket_recvfrom(conn->fd, conn->udp.rxbuf.buf, MAX_IOBUF_SIZE, &conn->udp.peer.ss, &conn->udp.peer.sslen);

        if (n == -1) {
            if ((errno != EAGAIN || errno != EWOULDBLOCK)) {
                conn->h->on_close(conn, strerror(errno));
            }
            return;
        }
        conn->h->on_read(conn, conn->udp.rxbuf.buf, n);
    }
    return;
}

void platform_connection_send(cdk_net_conn_t* conn)
{
    if (conn->type == SOCK_STREAM) {
        while (!cdk_list_empty(&(conn->tcp.txlist))) {

            cdk_txlist_node_t* e = cdk_list_data(cdk_list_head(&(conn->tcp.txlist)), cdk_txlist_node_t, n);
            while (e->off < e->len) {
                ssize_t n = platform_socket_send(conn->fd, e->buf + e->off, (int)(e->len - e->off));
                if (n == -1) {
                    if ((errno != EAGAIN || errno != EWOULDBLOCK)) {
                        conn->h->on_close(conn, strerror(errno));
                    }
                    return;
                }
                e->off += n;
            }
            conn->h->on_write(conn, e->buf, e->len);
            cdk_list_remove(&(e->n));
            free(e);
            e = NULL;
        }
    }
    if (conn->type == SOCK_DGRAM) {
        while (!cdk_list_empty(&(conn->udp.txlist))) {

            cdk_txlist_node_t* e = cdk_list_data(cdk_list_head(&(conn->udp.txlist)), cdk_txlist_node_t, n);
            ssize_t n = platform_socket_sendto(conn->fd, e->buf, (int)e->len, &(conn->udp.peer.ss), conn->udp.peer.sslen);
            if (n == -1) {
                if ((errno != EAGAIN || errno != EWOULDBLOCK)) {
                    conn->h->on_close(conn, strerror(errno));
                }
                return;
            }
            conn->h->on_write(conn, e->buf, e->len);
            cdk_list_remove(&(e->n));
            free(e);
            e = NULL;
        }
    }
    return;
}

void platform_connection_accept(cdk_net_conn_t* conn) {
    cdk_sock_t cli = platform_socket_accept(conn->fd);
    if (cli == -1) {
        if ((errno != EAGAIN || errno != EWOULDBLOCK)) {
            conn->h->on_close(conn, strerror(errno));
        }
        return;
    }
    cdk_net_conn_t* nconn = cdk_connection_create(platform_poller_retrieve(false), cli, PLATFORM_EVENT_R, conn->h);
    conn->h->on_accept(nconn);
    return;
}

void platform_connection_connect(cdk_net_conn_t* conn) {
    int err;
    socklen_t len;
    len = sizeof(int);

    getsockopt(conn->fd, SOL_SOCKET, SO_ERROR, &err, &len);
    if (err) {
        conn->h->on_close(conn, strerror(err));
    }
    else {
        conn->tcp.connected = true;
        conn->h->on_connect(conn);
    }
}