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

#define TCPv4_MSS       536
#define TCPv6_MSS       1220

void platform_socket_nonblock(cdk_sock_t sock) {

    int flag = fcntl(sock, F_GETFL, 0);
    if (flag == -1) {
        abort();
    }
    if (fcntl(sock, F_SETFL, flag | O_NONBLOCK)) {
        abort();
    }
}

void platform_socket_set_recvbuf(cdk_sock_t sock, int val) {

    if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (const void*)&val, sizeof(int))) {
        abort();
    }
}

void platform_socket_set_sendbuf(cdk_sock_t sock, int val) {

    if (setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (const void*)&val, sizeof(int))) {
        abort();
    }
}

void platform_socket_close(cdk_sock_t sock) {
    close(sock);
}

cdk_sock_t platform_socket_accept(cdk_sock_t sock) {

    cdk_sock_t cli;
    do {
        cli = accept(sock, NULL, NULL);
    } while (cli == INVALID_SOCKET && errno == EINTR);

    if (cli == INVALID_SOCKET) {

        if (errno != EAGAIN || errno != EWOULDBLOCK) {

            platform_socket_close(sock);
            abort();
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return INVALID_SOCKET;
        }
    }
    platform_socket_nonblock(cli);
    return cli;
}

void platform_socket_nodelay(cdk_sock_t sock, bool on) {

    int val = on ? 1 : 0;
    if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const void*)&val, sizeof(val))) {
        abort();
    }
}

void platform_socket_reuse_addr(cdk_sock_t sock) {

    int on = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void*)&on, sizeof(on))) {
        abort();
    }
}

#if defined(__linux__)
int platform_socket_af(cdk_sock_t sock) {

    int af;
    socklen_t len;

    len = sizeof(int);
    if (getsockopt(sock, SOL_SOCKET, SO_DOMAIN, &af, &len)) {
        abort();
    }
    return af;
}
#endif

#if defined(__APPLE__)
int platform_socket_af(cdk_sock_t sock) {

    struct sockaddr_storage ss;
    socklen_t len;

    len = sizeof(struct sockaddr_storage);
    getsockname(sock, (struct sockaddr*)&ss, &len);

    return ss.ss_family;
}
#endif

#if defined(__linux__)

void platform_socket_keepalive(cdk_sock_t sock) {

    int on = 1;
    int d = 60;
    int i = 1;  /** 1 second; same as default on win32 */
    int c = 10; /** 10 retries; same as hardcoded on win32 since vista */

    if (setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (const void*)&on, sizeof(on))) {
        abort();
    }
    if (setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, (const void*)&d, sizeof(d))) {
        abort();
    }
    if (setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, (const void*)&i, sizeof(i))) {
        abort();
    }
    if (setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, (const void*)&c, sizeof(c))) {
        abort();
    }
}

void platform_socket_maxseg(cdk_sock_t sock) {

    int af = platform_socket_af(sock);
    int mss = af == AF_INET ? TCPv4_MSS : TCPv6_MSS;

    if (setsockopt(sock, IPPROTO_TCP, TCP_MAXSEG, (const void*)&mss, sizeof(int))) {
        abort();
    }
}
#endif

#if defined(__APPLE__)

void platform_socket_keepalive(cdk_sock_t sock) {

    int on = 1;
    int d = 60;
    int i = 1;  /* 1 second; same as default on win32 */
    int c = 10; /* 10 retries; same as hardcoded on win32 since vista */

    if (setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (const void*)&on, sizeof(on))) {
        abort();
    }
    if (setsockopt(sock, IPPROTO_TCP, TCP_KEEPALIVE, (const void*)&d, sizeof(d))) {
        abort();
    }
    if (setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, (const void*)&i, sizeof(i))) {
        abort();
    }
    if (setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, (const void*)&c, sizeof(c))) {
        abort();
    }
}

void platform_socket_maxseg(cdk_sock_t sock) {

    /**
     * on macos, TCP_NOOPT seems to be the only way to restrict MSS to the minimum.
     * it strips all options out of the SYN packet which forces the remote party to fall back to the minimum MSS.
     * TCP_MAXSEG doesn't seem to work correctly for outbound connections on macOS/iOS.
     */
    int val = 1;
    if (setsockopt(sock, IPPROTO_TCP, TCP_NOOPT, (const void*)&val, sizeof(int))) {
        abort();
    }
}
#endif

void platform_socket_reuse_port(cdk_sock_t sock) {

    int on = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (const void*)&on, sizeof(on))) {
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
        platform_socket_reuse_port(sock);

        if (bind(sock, rp->ai_addr, rp->ai_addrlen) == -1) {
            platform_socket_close(sock);
            continue;
        }
        /**
         * these options inherited by connection-socket.
         */
        if (protocol == SOCK_STREAM) {
            if (listen(sock, SOMAXCONN) == -1) {
                platform_socket_close(sock);
                continue;
            }
            platform_socket_maxseg(sock);
            /**
             * must be after _tcp_maxseg. due to _tcp_maxseg set TCP_NOOPT on macos.
             */
            platform_socket_nodelay(sock, true);
            platform_socket_keepalive(sock);
        }
        /**
         * this option not inherited by connection-socket.
         */
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

}

void platform_socket_cleanup(void) {

}

cdk_sock_t platform_socket_dial(const char* restrict host, const char* restrict port, int protocol, bool* connected) {

    int ret;
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
        platform_socket_nonblock(sock);

        if (protocol == SOCK_STREAM) {
            platform_socket_maxseg(sock);
            platform_socket_nodelay(sock, true);
            platform_socket_keepalive(sock);

            do {
                ret = connect(sock, rp->ai_addr, rp->ai_addrlen);
            } while (ret == -1 && errno == EINTR);
            if (ret == -1) {
                if (errno != EINPROGRESS) {
                    platform_socket_close(sock);
                    continue;
                }
                if (errno == EINPROGRESS) {
                    break;
                }
            }
            else {
                *connected = true;
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

int platform_socket_socktype(cdk_sock_t sock) {

    int socktype;
    socklen_t len;

    len = sizeof(int);
    if (getsockopt(sock, SOL_SOCKET, SO_TYPE, &socktype, (socklen_t*)&len)) {
        perror("error");
        abort();
    }
    return socktype;
}

ssize_t platform_socket_recv(cdk_sock_t sock, void* buf, int size) {

    ssize_t n;
    do {
        n = recv(sock, buf, size, 0);
    } while (n == -1 && errno == EINTR);

    return n;
}

ssize_t platform_socket_send(cdk_sock_t sock, void* buf, int size) {

    ssize_t n;
    do {
        n = send(sock, buf, size, 0);
    } while (n == -1 && errno == EINTR);

    return n;
}

ssize_t platform_socket_recvfrom(cdk_sock_t sock, void* buf, int size, struct sockaddr_storage* ss, socklen_t* lenptr) {

    ssize_t n;
    do {
        n = recvfrom(sock, buf, size, 0, (struct sockaddr*)ss, lenptr);
    } while (n == -1 && errno == EINTR);

    return n;
}

ssize_t platform_socket_sendto(cdk_sock_t sock, void* buf, int size, struct sockaddr_storage* ss, socklen_t len) {

    ssize_t n;
    do {
        n = sendto(sock, buf, size, 0, (struct sockaddr*)ss, len);
    } while (n == -1 && errno == EINTR);

    return n;
}

int platform_socket_socketpair(int domain, int type, int protocol, cdk_sock_t socks[2]) {
    return socketpair(AF_LOCAL, type, protocol, socks);
}

char* platform_socket_error2string(int error) {
    return strerror(error);
}