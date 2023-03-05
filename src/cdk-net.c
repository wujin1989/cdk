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

#include "cdk/cdk-net.h"
#include "cdk/cdk-thread.h"
#include "cdk/cdk-queue.h"
#include "cdk/cdk-threadpool.h"
#include "cdk/cdk-sysinfo.h"
#include "cdk/cdk-memory.h"
#include "cdk/cdk-atomic.h"

#if defined(__linux__) || defined(__APPLE__)
#include "unix/unix-net.h"
#include "unix/unix-poller.h"
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#endif

#if defined(_WIN32)
#include "win/win-net.h"
#include <WS2tcpip.h>
#endif

#define TCPv4_SAFE_MSS    536
#define TCPv6_SAFE_MSS    1220

#define SNDRCV_BUFFER_SIZE_MIN  32767
#define SNDRCV_BUFFER_SIZE_STEP 16384

static void _inet_ntop(int af, const void* restrict s, char* restrict d) {

    if (af == AF_INET) {
        inet_ntop(af, s, d, INET_ADDRSTRLEN);
    }
    if (af == AF_INET6) {
        inet_ntop(af, s, d, INET6_ADDRSTRLEN);
    }
}

void cdk_net_rbuf(sock_t s, int v) {

    int r;
    for (;;) {
        r = setsockopt(s, SOL_SOCKET, SO_RCVBUF, (char*)&v, sizeof(int));
        if (r == 0) { break; }
        else if (v <= SNDRCV_BUFFER_SIZE_MIN) { break; }
        else if (v - SNDRCV_BUFFER_SIZE_STEP <= SNDRCV_BUFFER_SIZE_MIN) { v = SNDRCV_BUFFER_SIZE_MIN; }
        else { v -= SNDRCV_BUFFER_SIZE_STEP; }
    }
}

void cdk_net_sbuf(sock_t s, int v) {

    int r;
    for (;;) {
        r = setsockopt(s, SOL_SOCKET, SO_SNDBUF, (char*)&v, sizeof(int));
        if (r == 0) { break; }
        else if (v <= SNDRCV_BUFFER_SIZE_MIN) { break; }
        else if (v - SNDRCV_BUFFER_SIZE_STEP <= SNDRCV_BUFFER_SIZE_MIN) { v = SNDRCV_BUFFER_SIZE_MIN; }
        else { v -= SNDRCV_BUFFER_SIZE_STEP; }
    }
}

void cdk_net_inet_ntop(struct sockaddr_storage* ss, addrinfo_t* ai) {

    char d[MAX_ADDRSTRLEN];
    memset(d, 0, sizeof(d));

    switch (ss->ss_family)
    {
    case AF_INET:
    {
        struct sockaddr_in* si = (struct sockaddr_in*)ss;

        _inet_ntop(AF_INET, &si->sin_addr, d);
        ai->p = ntohs(si->sin_port);
        ai->f = AF_INET;
        break;
    }
    case AF_INET6:
    {
        struct sockaddr_in6* si6 = (struct sockaddr_in6*)ss;

        _inet_ntop(AF_INET6, &si6->sin6_addr, d);
        ai->p = ntohs(si6->sin6_port);
        ai->f = AF_INET6;
        break;
    }
    default:
        return;
    }

    memcpy(ai->a, d, MAX_ADDRSTRLEN);
}

void cdk_net_inet_pton(addrinfo_t* ai, struct sockaddr_storage* ss) {

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

void cdk_net_obtain_addr(sock_t s, addrinfo_t* ai, bool p) {

    struct sockaddr_storage ss;
    socklen_t               l;

    l = sizeof(struct sockaddr_storage);

    if (p) { getpeername(s, (struct sockaddr*)&ss, &l); }
    if (!p) { getsockname(s, (struct sockaddr*)&ss, &l); }

    cdk_net_inet_ntop(&ss, ai);
}

int cdk_net_af(sock_t s) {

    return _net_af(s);
}

int cdk_net_socktype(sock_t s) {

    return _net_socktype(s);
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