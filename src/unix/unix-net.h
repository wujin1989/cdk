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

_Pragma("once")

#include "cdk/cdk-types.h"

extern sock_t  _tcp_accept(sock_t s);
extern void    _tcp_nodelay(sock_t s, bool on);
extern void    _tcp_keepalive(sock_t s);
extern void    _tcp_maxseg(sock_t s);
extern void    _net_nonblock(sock_t s);
extern void    _net_reuse_addr(sock_t s);
extern void    _net_reuse_port(sock_t s);
extern sock_t  _net_listen(const char* restrict h, const char* restrict p, int t);
extern sock_t  _net_dial(const char* restrict h, const char* restrict p, int t);
extern void    _net_close(sock_t s);
extern int     _net_af(sock_t s);
extern int     _net_socktype(sock_t s);
extern ssize_t _net_recv(sock_t s, void* buf, size_t len);
extern ssize_t _net_send(sock_t s, void* buf, size_t len);
extern ssize_t _net_recvfrom(sock_t s, void* buf, size_t len, struct sockaddr_storage* ss, socklen_t* lp);
extern ssize_t _net_sendto(sock_t s, void* buf, size_t len, struct sockaddr_storage* ss, socklen_t l);


