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

#include "win-net.h"
#include <WS2tcpip.h>

/* ///////////////////////////////////////////  private  //////////////////////////////////////////////////////////// */

static void _nodelay(sock_t s, bool on) {

    int v;
    if (on) { v = 1; }
    else { v = 0; }
    setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (const void*)&v, sizeof(v));
}

static void _maxseg(sock_t s) {

    int    v;
    int    af;
    int    r;

    af = _cdk_net_af(s);
    /**
     * windows doesn't support setting TCP_MAXSEG but IP_PMTUDISC_DONT forces the MSS to the protocol
     * minimum which is what we want here. linux doesn't do this (disabling PMTUD just avoids setting DF).
     */
    if (af == AF_INET) {

        v = IP_PMTUDISC_DONT;
        r = setsockopt(s, IPPROTO_IP, IP_MTU_DISCOVER, (char*)&v, sizeof(int));
        if (r == SOCKET_ERROR) { abort(); }
    }
    if (af == AF_INET6) {

        v = IP_PMTUDISC_DONT;
        r = setsockopt(s, IPPROTO_IPV6, IPV6_MTU_DISCOVER, (char*)&v, sizeof(int));
        if (r == SOCKET_ERROR) { abort(); }
    }
}

static void _reuse_addr(sock_t s) {

    int r;
    int on = 1;
    r = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const void*)&on, sizeof(on));
    if (r == SOCKET_ERROR) { abort(); }
}

static sock_t _listen(const char* restrict h, const char* restrict p, int t) {

    int                r;
    WSADATA            d;
    SOCKET             s;
    struct addrinfo    hints;
    struct addrinfo*   res;
    struct addrinfo*   rp;

    r = WSAStartup(MAKEWORD(2, 2), &d);
    if (r != NO_ERROR) { return INVALID_SOCKET; }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family     = AF_UNSPEC;
    hints.ai_socktype   = t;
    hints.ai_flags      = AI_PASSIVE;
    hints.ai_protocol   = 0;
    hints.ai_canonname  = NULL;
    hints.ai_addr       = NULL;
    hints.ai_next       = NULL;

    r = getaddrinfo(h, p, &hints, &res);
    if (r != 0) { return INVALID_SOCKET; }

    for (rp = res; rp != NULL; rp = rp->ai_next) {
        s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (s == INVALID_SOCKET) { continue; }

        _reuse_addr(s);

        if (bind(s, rp->ai_addr, (int)rp->ai_addrlen) == SOCKET_ERROR) { closesocket(s); continue; }
        if (t == SOCK_STREAM) {
            if (listen(s, SOMAXCONN) < 0) { closesocket(s); continue; }
            _maxseg(s);
        }
        break;
    }
    if (rp == NULL) { return INVALID_SOCKET; }
    freeaddrinfo(res);

    return s;
}

static sock_t _dial(const char* restrict h, const char* restrict p, int t) {
    int                r;
    WSADATA            d;
    SOCKET             s;
    struct addrinfo    hints;
    struct addrinfo*   res;
    struct addrinfo*   rp;

    r = WSAStartup(MAKEWORD(2, 2), &d);
    if (r != NO_ERROR) { return INVALID_SOCKET; }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family     = AF_UNSPEC;
    hints.ai_socktype   = t;
    hints.ai_flags      = 0;
    hints.ai_protocol   = 0;
    hints.ai_canonname  = NULL;
    hints.ai_addr       = NULL;
    hints.ai_next       = NULL;

    r = getaddrinfo(h, p, &hints, &res);
    if (r != 0) { return INVALID_SOCKET; }

    for (rp = res; rp != NULL; rp = rp->ai_next) {

        s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (s == INVALID_SOCKET) { continue; }

        if (t == SOCK_STREAM) {
            _maxseg(s);
        }
        if (connect(s, rp->ai_addr, (int)rp->ai_addrlen) == SOCKET_ERROR) { 
            closesocket(s); continue; 
        }
        
        _nodelay(s, true);
        break;
    }
    if (rp == NULL) { return INVALID_SOCKET; }
    freeaddrinfo(res);
    return s;
}

/* ///////////////////////////////////////////  common  //////////////////////////////////////////////////////////// */

void _cdk_net_close(sock_t s) {

    closesocket(s);
}

void _cdk_net_rtimeo(sock_t s, int t) {

    int r;
    r = setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&t, sizeof(t));
    if (r == SOCKET_ERROR) { abort(); }
}

void _cdk_net_stimeo(sock_t s, int t) {

    int r;
    r = setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char*)&t, sizeof(t));
    if (r == SOCKET_ERROR) { abort(); }
}

int _cdk_net_af(sock_t s) {

    WSAPROTOCOL_INFOW i; /* using unicode name to avoiding ninja build warning */
    socklen_t         l;

    l = sizeof(WSAPROTOCOL_INFOW);
    getsockopt(s, SOL_SOCKET, SO_PROTOCOL_INFO, (char*)&i, &l);

    return i.iAddressFamily;
}

/* ///////////////////////////////////////////  tcp  //////////////////////////////////////////////////////////// */

sock_t _cdk_tcp_accept(sock_t s) {

    sock_t c = accept(s, NULL, NULL);
    if (c == INVALID_SOCKET) { abort(); }

    _nodelay(c, true);
    return c;
}

sock_t _cdk_tcp_listen(const char* restrict h, const char* restrict p) {
	
    return _listen(h, p, SOCK_STREAM);
}

void _cdk_tcp_keepalive(sock_t s) {

    int r;
    int on = 1;
    int d  = 60;
    int i  = 1;  /* 1 second; same as default on win32 */
    int c  = 10; /* 10 retries; same as hardcoded on win32 since vista */

    r = setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (const void*)&on, sizeof(on));
    if (r == SOCKET_ERROR) { abort(); }
    r = setsockopt(s, IPPROTO_TCP, TCP_KEEPIDLE, (const void*)&d, sizeof(d));
    if (r == SOCKET_ERROR) { abort(); }
    r = setsockopt(s, IPPROTO_TCP, TCP_KEEPINTVL, (const void*)&i, sizeof(i));
    if (r == SOCKET_ERROR) { abort(); }
    r = setsockopt(s, IPPROTO_TCP, TCP_KEEPCNT, (const void*)&c, sizeof(c));
    if (r == SOCKET_ERROR) { abort(); }
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