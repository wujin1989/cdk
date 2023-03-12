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

#include "platform-socket.h"

static void cdk_net_inet_ntop(int af, const void* restrict src, char* restrict dst) {

    if (af == AF_INET) {
        inet_ntop(af, src, dst, INET_ADDRSTRLEN);
    }
    if (af == AF_INET6) {
        inet_ntop(af, src, dst, INET6_ADDRSTRLEN);
    }
}

void cdk_net_ntop(struct sockaddr_storage* ss, cdk_addrinfo_t* ai) {

    char d[MAX_ADDRSTRLEN];
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

    memcpy(ai->a, d, MAX_ADDRSTRLEN);
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

void cdk_net_recvbuf(cdk_sock_t sock, int val) {
    
    platform_socket_recvbuf(sock, val);
}

void cdk_net_sendbuf(cdk_sock_t sock, int val) {

    platform_socket_sendbuf(sock, val);
}








// num = 0, auto.
void cdk_net_concurrent_slaves(int64_t num) {

    _poller_concurrent_slaves(num);
}

poller_conn_t* cdk_net_listen(const char* restrict t, const char* restrict h, const char* restrict p, poller_handler_t* handler) {

    _poller_create();
    return _poller_listen(t, h, p, handler);
}

poller_conn_t* cdk_net_dial(const char* restrict t, const char* restrict h, const char* restrict p, poller_handler_t* handler) {

    _poller_create();
    return _poller_dial(t, h, p, handler);
}

void cdk_net_poll(void) {

    _poller_master();
    _poller_destroy();

    return;
}

void cdk_net_postrecv(poller_conn_t* conn) {

    _poller_postrecv(conn);
    return;
}

void cdk_net_postsend(poller_conn_t* conn, void* data, size_t size) {

    _poller_postsend(conn, data, size);
    return;
}

void cdk_net_setup_splicer(poller_conn_t* conn, splicer_profile_t* splicer) {

    _poller_setup_splicer(conn, splicer);
}

void cdk_net_close(poller_conn_t* conn) {

    _poller_conn_destroy(conn);
}