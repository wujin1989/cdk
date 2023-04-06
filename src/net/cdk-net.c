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
#include "net/cdk-net-connection.h"
#include "cdk/container/cdk-list.h"

static void cdk_net_inet_ntop(int af, const void* restrict src, char* restrict dst) {
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

        cdk_net_inet_ntop(AF_INET, &si->sin_addr, d);
        ai->p = ntohs(si->sin_port);
        ai->f = AF_INET;
        break;
    }
    case AF_INET6:
    {
        struct sockaddr_in6* si6 = (struct sockaddr_in6*)ss;

        cdk_net_inet_ntop(AF_INET6, &si6->sin6_addr, d);
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

    if (!strncmp(type, "tcp", strlen("tcp")))
    {
        sock = platform_socket_listen(host, port, SOCK_STREAM);
        conn = cdk_net_connection_create(platform_poller_retrieve(true), sock, PLATFORM_EVENT_A, handler);
    }
    if (!strncmp(type, "udp", strlen("udp")))
    {
        sock = platform_socket_listen(host, port, SOCK_DGRAM);
        conn = cdk_net_connection_create(platform_poller_retrieve(false), sock, PLATFORM_EVENT_R, handler);
    }
    return conn;
}

cdk_net_conn_t* cdk_net_dial(const char* type, const char* host, const char* port, cdk_net_handler_t* handler)
{
    cdk_sock_t sock;
    cdk_net_conn_t* conn;
    cdk_addrinfo_t ai;
    struct sockaddr_storage ss;

    platform_socket_startup();
    platform_poller_create();

    memset(&ai, 0, sizeof(cdk_addrinfo_t));
    memset(&ss, 0, sizeof(struct sockaddr_storage));

    if (!strncmp(type, "tcp", strlen("tcp"))) {

        sock = platform_socket_dial(host, port, SOCK_STREAM);
        conn = cdk_net_connection_create(platform_poller_retrieve(false), sock, PLATFORM_EVENT_C, handler);
    }
    if (!strncmp(type, "udp", strlen("udp"))) {

        sock = platform_socket_dial(host, port, SOCK_DGRAM);
        conn = cdk_net_connection_create(platform_poller_retrieve(false), sock, PLATFORM_EVENT_W, handler);

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

    platform_poller_poll(platform_poller_retrieve(true));
    platform_poller_destroy();
    platform_socket_cleanup();
}

void cdk_net_postrecv(cdk_net_conn_t* conn) {
    cdk_net_connection_postrecv(conn);
}

void cdk_net_postsend(cdk_net_conn_t* conn, void* data, size_t size) {
    inner_offset_buf_t* buffer = malloc(sizeof(inner_offset_buf_t) + size);
    if (buffer) {
        memset(buffer, 0, sizeof(inner_offset_buf_t) + size);
        memcpy(buffer->buf, data, size);
        buffer->len = size;
        buffer->off = 0;

        mtx_lock(&conn->txmtx);
        if (conn->type == SOCK_STREAM) {
            cdk_list_insert_tail(&(conn->tcp.txlist), &(buffer->n));
        }
        if (conn->type == SOCK_DGRAM) {
            cdk_list_insert_tail(&(conn->udp.txlist), &(buffer->n));
        }
        cdk_net_connection_postsend(conn);
        mtx_unlock(&conn->txmtx);
    }
}

void cdk_net_close(cdk_net_conn_t* conn) {
    cdk_net_connection_destroy(conn);
}