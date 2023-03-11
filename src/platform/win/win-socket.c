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

#include "cdk/cdk-types.h"
#include "platform-socket.h"
#include <WS2tcpip.h>

void platform_socket_nonblock(cdk_sock_t sock) {

    u_long on = 1;
    if (ioctlsocket(sock, FIONBIO, &on)) {
        abort();
    }
}

cdk_sock_t platform_socket_tcp_accept(cdk_sock_t sock) {

    cdk_sock_t cli = accept(sock, NULL, NULL);

    if (cli == INVALID_SOCKET) {

        if (WSAGetLastError() != WSAEWOULDBLOCK) {
            
            platform_socket_close(sock);
            abort();
        }
        if (WSAGetLastError() == WSAEWOULDBLOCK) {
            return INVALID_SOCKET;
        }
    }
    platform_socket_nonblock(cli);
    return cli;
}

void platform_socket_tcp_nodelay(cdk_sock_t sock, bool on) {

    int val = on ? 1 : 0;
    if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&val, sizeof(val))) {
        abort();
    }
}

void platform_socket_tcp_keepalive(cdk_sock_t sock) {

    int on = 1;
    int d = 60;
    int i = 1;  /* 1 second; same as default on win32 */
    int c = 10; /* 10 retries; same as hardcoded on win32 since vista */

    if (setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (const char*)&on, sizeof(on))) {
        abort();
    }
    if (setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, (const char*)&d, sizeof(d))) {
        abort();
    }
    if (setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, (const char*)&i, sizeof(i))) {
        abort();
    }
    if (setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, (const char*)&c, sizeof(c))) {
        abort();
    }
}

void platform_socket_tcp_maxseg(cdk_sock_t sock) {

    int af = platform_socket_af(sock);
    /**
     * windows doesn't support setting TCP_MAXSEG but IP_PMTUDISC_DONT forces the MSS to the protocol
     * minimum which is what we want here. linux doesn't do this (disabling PMTUD just avoids setting DF).
     */
    if (af == AF_INET) {

        int val = IP_PMTUDISC_DONT;
        if (setsockopt(sock, IPPROTO_IP, IP_MTU_DISCOVER, (const char*)&val, sizeof(int))) {
            abort();
        }
    }
    if (af == AF_INET6) {

        int val = IP_PMTUDISC_DONT;
        if (setsockopt(sock, IPPROTO_IPV6, IPV6_MTU_DISCOVER, (const char*)&val, sizeof(int))) {
            abort();
        }
    }
}

void platform_socket_reuse_addr(cdk_sock_t sock) {

    int on = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on))) {
        abort();
    }
}

cdk_sock_t platform_socket_listen(const char* restrict host, const char* restrict port, int protocol) {

    cdk_sock_t sock;
    struct addrinfo  hints;
    struct addrinfo* res;
    struct addrinfo* rp;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family    = AF_UNSPEC;
    hints.ai_socktype  = protocol;
    hints.ai_flags     = AI_PASSIVE;
    hints.ai_protocol  = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr      = NULL;
    hints.ai_next      = NULL;

    if (getaddrinfo(host, port, &hints, &res)) {
        abort();
    }
    for (rp = res; rp != NULL; rp = rp->ai_next) {
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock == INVALID_SOCKET) {
            continue;
        }
        platform_socket_reuse_addr(sock);

        if (bind(sock, rp->ai_addr, (int)rp->ai_addrlen) == SOCKET_ERROR) {
            platform_socket_close(sock);
            continue;
        }
        if (protocol == SOCK_STREAM) {
            if (listen(sock, SOMAXCONN) == SOCKET_ERROR) {
                platform_socket_close(sock);
                continue;
            }
            platform_socket_tcp_maxseg(sock);
            platform_socket_tcp_nodelay(sock, true);
            platform_socket_tcp_keepalive(sock);
        }
        platform_socket_nonblock(sock);
        break;
    }
    if (rp == NULL) {
        abort();
    }
    freeaddrinfo(res);
    return sock;
}

void platform_socket_startup(void) {

    WSADATA  d;
    if (WSAStartup(MAKEWORD(2, 2), &d)) {
        abort();
    }
}

void platform_socket_cleanup(void) {

    WSACleanup();
}

cdk_sock_t  platform_socket_dial(const char* restrict host, const char* restrict port, int protocol) {

    cdk_sock_t sock;
    struct addrinfo  hints;
    struct addrinfo* res;
    struct addrinfo* rp;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family    = AF_UNSPEC;
    hints.ai_socktype  = protocol;
    hints.ai_flags     = 0;
    hints.ai_protocol  = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr      = NULL;
    hints.ai_next      = NULL;

    if (getaddrinfo(host, port, &hints, &res)) {
        abort();
    }
    for (rp = res; rp != NULL; rp = rp->ai_next) {

        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock == INVALID_SOCKET) {
            continue;
        }
        if (protocol == SOCK_STREAM) {
            platform_socket_tcp_maxseg(sock);
            platform_socket_tcp_nodelay(sock, true);
            platform_socket_tcp_keepalive(sock);
        }
        platform_socket_nonblock(sock);

        if (connect(sock, rp->ai_addr, (int)rp->ai_addrlen)) {

            if (WSAGetLastError() != WSAEWOULDBLOCK) {
                platform_socket_close(sock);
                continue;
            }
            if (WSAGetLastError() == WSAEWOULDBLOCK) {
                break;
            }
        }
        break;
    }
    if (rp == NULL) {
        abort();
    }
    freeaddrinfo(res);
    return sock;
}

void platform_socket_close(cdk_sock_t sock) {

    closesocket(sock);
}

int platform_socket_af(cdk_sock_t sock) {

    WSAPROTOCOL_INFOW info; /* using unicode name to avoiding ninja build warning */
    socklen_t len;

    len = sizeof(WSAPROTOCOL_INFOW);
    if (getsockopt(sock, SOL_SOCKET, SO_PROTOCOL_INFO, (char*)&info, &len)) {
        abort();
    }

    return info.iAddressFamily;
}

int platform_socket_socktype(cdk_sock_t sock) {

    int socktype;
    int len = sizeof(int);

    if (getsockopt(sock, SOL_SOCKET, SO_TYPE, (char*)&socktype, &len)) {
        abort();
    }
    return socktype;
}

ssize_t platform_socket_recv(cdk_sock_t sock, void* buf, int size) {

    return recv(sock, buf, size, 0);
}

ssize_t platform_socket_send(cdk_sock_t sock, void* buf, int size) {

    return send(sock, buf, size, 0);
}

ssize_t platform_socket_recvfrom(cdk_sock_t sock, void* buf, int size, struct sockaddr_storage* ss, socklen_t* lenptr) {

    return recvfrom(sock, buf, size, 0, (struct sockaddr*)ss, lenptr);
}

ssize_t platform_socket_sendto(cdk_sock_t sock, void* buf, int size, struct sockaddr_storage* ss, socklen_t len) {

    return sendto(sock, buf, size, 0, (struct sockaddr*)ss, len);
}