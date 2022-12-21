/** Copyright (c) 2022, Wu Jin <wujin.developer@gmail.com>
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

#if defined(__linux__)
#define _GNU_SOURCE
#endif

#include "unix-net.h"
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>

#define TCPv4_MSS       536
#define TCPv6_MSS       1220

/* ///////////////////////////////////////////  private  //////////////////////////////////////////////////////////// */

static void _nonblock(sock_t s) {

    int flag = fcntl(s, F_GETFL, 0);
    if (-1 == flag) {
        abort();
    }
    fcntl(s, F_SETFL, flag | O_NONBLOCK);
}

static int _wait(sock_t s, int t) {

    struct pollfd pfd;
    memset(&pfd, 0, sizeof(struct pollfd));

    pfd.fd     = s;
    pfd.events = POLLOUT;
    
    return poll(&pfd, 1, t);
}

static sock_t _tcp_accept(sock_t s) {

    sock_t c;
    do {
        c = accept(s, NULL, NULL);
    } while (c == -1 && errno == EINTR);
    
    if (c == -1) {
        close(s);
        return -1;
    }
    _nonblock(c);
    return c;
}

static void _nodelay(sock_t s, bool on) {

    int v;
    if (on) { v = 1; }
    else { v = 0; }
    setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (const void*)&v, sizeof(v));
}

static void _reuse_addr(sock_t s) {

    int r;
    int on = 1;
    r = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const void*)&on, sizeof(on));
    if (r < 0) { abort(); }
}

#if defined(__linux__)

static void _keepalive(sock_t s) {

    int r;
    int on = 1;
    int d = 60;
    int i = 1;  /* 1 second; same as default on win32 */
    int c = 10; /* 10 retries; same as hardcoded on win32 since vista */

    r = setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (const void*)&on, sizeof(on));
    if (r < 0) { abort(); }
    r = setsockopt(s, IPPROTO_TCP, TCP_KEEPIDLE, (const void*)&d, sizeof(d));
    if (r < 0) { abort(); }
    r = setsockopt(s, IPPROTO_TCP, TCP_KEEPINTVL, (const void*)&i, sizeof(i));
    if (r < 0) { abort(); }
    r = setsockopt(s, IPPROTO_TCP, TCP_KEEPCNT, (const void*)&c, sizeof(c));
    if (r < 0) { abort(); }
}

static void _maxseg(sock_t s) {

    int    v;
    int    af;
    int    r;

    af = _cdk_net_af(s);
    v = af == AF_INET ? TCPv4_MSS : TCPv6_MSS;

    r = setsockopt(s, IPPROTO_TCP, TCP_MAXSEG, (char*)&v, sizeof(int));
    if (r < 0) { abort(); }
}
#endif

#if defined(__APPLE__)

static void _keepalive(sock_t s) {

    int r;
    int on = 1;
    int d = 60;
    int i = 1;  /* 1 second; same as default on win32 */
    int c = 10; /* 10 retries; same as hardcoded on win32 since vista */

    r = setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (const void*)&on, sizeof(on));
    if (r < 0) { abort(); }
    r = setsockopt(s, IPPROTO_TCP, TCP_KEEPALIVE, (const void*)&d, sizeof(d));
    if (r < 0) { abort(); }
    r = setsockopt(s, IPPROTO_TCP, TCP_KEEPINTVL, (const void*)&i, sizeof(i));
    if (r < 0) { abort(); }
    r = setsockopt(s, IPPROTO_TCP, TCP_KEEPCNT, (const void*)&c, sizeof(c));
    if (r < 0) { abort(); }
}

static void _maxseg(sock_t s) {

    /**
     * on macos, TCP_NOOPT seems to be the only way to restrict MSS to the minimum.
     * it strips all options out of the SYN packet which forces the remote party to fall back to the minimum MSS.
     * TCP_MAXSEG doesn't seem to work correctly for outbound connections on macOS/iOS.
     */
    int r;
    int v = 1;
    r = setsockopt(s, IPPROTO_TCP, TCP_NOOPT, (char*)&v, sizeof(int));
    if (r < 0) { abort(); }
}
#endif

static void _reuse_port(sock_t s) {

    int r;
    int on = 1;
    r = setsockopt(s, SOL_SOCKET, SO_REUSEPORT, (const void*)&on, sizeof(on));
    if (r < 0) { abort(); }
}

static sock_t _listen(const char* restrict h, const char* restrict p, int t) {
    int                 r;
    int                 s;
    struct addrinfo     hints;
    struct addrinfo*    res;
    struct addrinfo*    rp;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family     = AF_UNSPEC;
    hints.ai_socktype   = t;
    hints.ai_flags      = AI_PASSIVE;
    hints.ai_protocol   = 0;
    hints.ai_canonname  = NULL;
    hints.ai_addr       = NULL;
    hints.ai_next       = NULL;

    r = getaddrinfo(h, p, &hints, &res);
    if (r != 0) { return -1; }

    for (rp = res; rp != NULL; rp = rp->ai_next) {
       
        s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (s == -1) { continue; }

        _reuse_addr(s);
        _reuse_port(s);

        if (bind(s, rp->ai_addr, rp->ai_addrlen) == -1) { close(s); continue; }
        /**
         * these options inherited by connection-socket. 
         */
        if (t == SOCK_STREAM) {
            if (listen(s, SOMAXCONN) == -1) {
                close(s);
                continue;
            }
            _maxseg(s);
            _nodelay(s, true);
            _keepalive(s);
        }
        /**
         * this option not inherited by connection-socket.
         */
        //_nonblock(s);
        break;
    }
    if (rp == NULL) { return -1; }
    freeaddrinfo(res);
    return s;
}

static sock_t _dial(const char* restrict h, const char* restrict p, int t) {
    int                 r;
    int                 s;
    struct addrinfo     hints;
    struct addrinfo*    res;
    struct addrinfo*    rp;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family     = AF_UNSPEC;
    hints.ai_socktype   = t;
    hints.ai_flags      = 0;
    hints.ai_protocol   = 0;
    hints.ai_canonname  = NULL;
    hints.ai_addr       = NULL;
    hints.ai_next       = NULL;

    r = getaddrinfo(h, p, &hints, &res);
    if (r != 0) { return -1; }

    for (rp = res; rp != NULL; rp = rp->ai_next) {

        s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (s == -1) { continue; }
        if (t == SOCK_STREAM) {
            _maxseg(s);
            _nodelay(s, true);
            _keepalive(s);
        }
        _nonblock(s);
        do {
            r = connect(s, rp->ai_addr, rp->ai_addrlen);
        } while (r == -1 && errno == EINTR);
        
        if (r == -1 && errno != EINPROGRESS) {
            close(s);
            continue;
        }
        if (r == -1 && errno == EINPROGRESS) {
            if (_wait(s, 10000) <= 0) {
                close(s);
                return -1;
            }
            else {
                int       e;
                socklen_t l;
                l = sizeof(int);
                getsockopt(s, SOL_SOCKET, SO_ERROR, (char*)&e, &l);
                if (e) {
                    close(s);
                    return -1;
                }
            }
        }
        break;
    }
    if (rp == NULL) { return -1; }
    freeaddrinfo(res);
    return s;
}

/* ///////////////////////////////////////////  common  //////////////////////////////////////////////////////////// */

void _cdk_net_close(sock_t s) {

    close(s);
}

#if defined(__linux__)
int _cdk_net_af(sock_t s) {

    int       d;
    socklen_t l;

    l = sizeof(int);
    getsockopt(s, SOL_SOCKET, SO_DOMAIN, (char*)&d, (socklen_t*)&l);

    return d;
}
#endif

#if defined(__APPLE__)
int _cdk_net_af(sock_t s) {

    struct sockaddr_storage ss;
    socklen_t               ssl;

    ssl = sizeof(struct sockaddr_storage);
    getsockname(s, (struct sockaddr*)&ss, &ssl);

    return ss.ss_family;
}
#endif

/* ///////////////////////////////////////////  tcp  //////////////////////////////////////////////////////////// */

sock_t _cdk_tcp_listen(const char* restrict h, const char* restrict p) {
	
    return _listen(h, p, SOCK_STREAM);
}

sock_t _cdk_tcp_dial(const char* restrict h, const char* restrict p) {
    
    return _dial(h, p, SOCK_STREAM);
}

/* ///////////////////////////////////////////  udp  //////////////////////////////////////////////////////////// */
sock_t _cdk_udp_listen(const char* restrict h, const char* restrict p) {

    return _listen(h, p, SOCK_DGRAM);
}

sock_t _cdk_udp_dial(const char* restrict h, const char* restrict p) {

    return _dial(h, p, SOCK_DGRAM);
}
