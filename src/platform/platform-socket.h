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

#if defined(__linux__) || defined(__APPLE__)
#define PLATFORM_SO_ERROR_EAGAIN EAGAIN
#define PLATFORM_SO_ERROR_EWOULDBLOCK EWOULDBLOCK
#define PLATFORM_SO_ERROR_ECONNRESET ECONNRESET
#define PLATFORM_SO_ERROR_ETIMEDOUT ETIMEDOUT
#define PLATFORM_SO_ERROR_INVALID_SOCKET -1
#define PLATFORM_SO_ERROR_SOCKET_ERROR -1
#endif

#if defined(_WIN32)
#define PLATFORM_SO_ERROR_EAGAIN WSAEWOULDBLOCK
#define PLATFORM_SO_ERROR_EWOULDBLOCK WSAEWOULDBLOCK
#define PLATFORM_SO_ERROR_ECONNRESET WSAECONNRESET
#define PLATFORM_SO_ERROR_ETIMEDOUT WSAETIMEDOUT
#define PLATFORM_SO_ERROR_INVALID_SOCKET INVALID_SOCKET
#define PLATFORM_SO_ERROR_SOCKET_ERROR SOCKET_ERROR
#endif

extern void       platform_socket_recvtimeo(cdk_sock_t sock, int timeout_ms);
extern void       platform_socket_sendtimeo(cdk_sock_t sock, int timeout_ms);
extern void       platform_socket_setrecvbuf(cdk_sock_t sock, int val);
extern void       platform_socket_setsendbuf(cdk_sock_t sock, int val);
extern void       platform_socket_nodelay(cdk_sock_t sock, bool on);
extern void       platform_socket_v6only(cdk_sock_t sock, bool on);
extern void       platform_socket_rss(cdk_sock_t sock, uint16_t idx, int cores);
extern void       platform_socket_keepalive(cdk_sock_t sock);
extern void       platform_socket_maxseg(cdk_sock_t sock);
extern void       platform_socket_nonblock(cdk_sock_t sock);
extern void       platform_socket_reuse_addr(cdk_sock_t sock);
extern void       platform_socket_reuse_port(cdk_sock_t sock);
extern int        platform_socket_extract_family(cdk_sock_t sock);
extern void       platform_socket_startup(void);
extern void       platform_socket_cleanup(void);
extern cdk_sock_t platform_socket_accept(cdk_sock_t sock, bool nonblocking);
extern cdk_sock_t platform_socket_listen(const char* restrict host, const char* restrict port, int protocol, int idx, int cores, bool nonblocking);
extern cdk_sock_t platform_socket_dial(const char* restrict host, const char* restrict port, int protocol, bool* connected, bool nonblocking);
extern void       platform_socket_close(cdk_sock_t sock);
extern int        platform_socket_getaddrfamily(cdk_sock_t sock);
extern int        platform_socket_getsocktype(cdk_sock_t sock);
extern ssize_t    platform_socket_recv(cdk_sock_t sock, void* buf, int size);
extern ssize_t    platform_socket_send(cdk_sock_t sock, void* buf, int size);
extern ssize_t    platform_socket_recvall(cdk_sock_t sock, void* buf, int size);
extern ssize_t    platform_socket_sendall(cdk_sock_t sock, void* buf, int size);
extern ssize_t    platform_socket_recvfrom(cdk_sock_t sock, void* buf, int size, struct sockaddr_storage* ss, socklen_t* lenptr);
extern ssize_t    platform_socket_sendto(cdk_sock_t sock, void* buf, int size, struct sockaddr_storage* ss, socklen_t len);
extern int          platform_socket_socketpair(int domain, int type, int protocol, cdk_sock_t socks[2]);
extern char*  platform_socket_error2string(int error);
extern int          platform_socket_lasterror(void);
extern cdk_pollfd_t platform_socket_pollfd_create(void);
extern void         platform_socket_pollfd_destroy(cdk_pollfd_t pfd);
