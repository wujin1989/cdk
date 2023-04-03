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
#include "cdk/container/cdk-rbtree.h"
#include "cdk/container/cdk-list.h"

#define MAX_IOBUF_SIZE 4096

cdk_net_conn_t* cdk_net_connection_create(cdk_poller_t* poller, cdk_sock_t sock, int cmd, cdk_net_handler_t* handler)
{
    cdk_net_conn_t* conn = malloc(sizeof(cdk_net_conn_t));

    if (conn) {
        conn->cmd = cmd;
        conn->fd = sock;
        conn->h = handler;
        conn->type = platform_socket_socktype(sock);
        conn->active = true;
        cdk_rbtree_init(&(conn->owners), RB_KEYTYPE_UINT32);
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

}

void cdk_net_connection_destroy(cdk_net_conn_t* conn) {

}

void cdk_net_connection_process(cdk_net_conn_t* conn) {

}