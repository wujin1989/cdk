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

_Pragma("once")

#include "cdk/cdk-types.h"

extern void        platform_socket_startup(void);
extern void        platform_socket_cleanup(void);
extern void        platform_socket_set_recvbuf(cdk_sock_t sock, int val);
extern void        platform_socket_set_sendbuf(cdk_sock_t sock, int val);
extern cdk_sock_t  platform_socket_accept(cdk_sock_t sock);
extern void        platform_socket_nodelay(cdk_sock_t sock, bool on);
extern void        platform_socket_keepalive(cdk_sock_t sock);
extern void        platform_socket_maxseg(cdk_sock_t sock);
extern void        platform_socket_nonblock(cdk_sock_t sock);
extern void        platform_socket_reuse_addr(cdk_sock_t sock);
extern void        platform_socket_reuse_port(cdk_sock_t sock);
extern cdk_sock_t  platform_socket_listen(const char* restrict host, const char* restrict port, int protocol);
extern cdk_sock_t  platform_socket_dial(const char* restrict host, const char* restrict port, int protocol);
extern void        platform_socket_close(cdk_sock_t sock);
extern int         platform_socket_af(cdk_sock_t sock);
extern int         platform_socket_socktype(cdk_sock_t sock);
extern ssize_t     platform_socket_recv(cdk_sock_t sock, void* buf, int size);
extern ssize_t     platform_socket_send(cdk_sock_t sock, void* buf, int size);
extern ssize_t     platform_socket_recvfrom(cdk_sock_t sock, void* buf, int size, struct sockaddr_storage* ss, socklen_t* lenptr);
extern ssize_t     platform_socket_sendto(cdk_sock_t sock, void* buf, int size, struct sockaddr_storage* ss, socklen_t len);


