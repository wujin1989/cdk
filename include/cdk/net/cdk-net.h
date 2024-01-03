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

/** common */
extern void cdk_net_ntop(struct sockaddr_storage* ss, cdk_addrinfo_t* ai);
extern void cdk_net_pton(cdk_addrinfo_t* ai, struct sockaddr_storage* ss);
extern void cdk_net_getaddrinfo(cdk_sock_t sock, cdk_addrinfo_t* ai, bool peer);
extern int cdk_net_getsocktype(cdk_sock_t sock);
/** async */
extern void cdk_net_listen(const char* protocol, const char* host, const char* port, cdk_handler_t* handler);
extern void cdk_net_dial(const char* protocol, const char* host, const char* port, cdk_handler_t* handler);
extern void cdk_net_send(cdk_channel_t* channel, void* data, size_t size);
extern void cdk_net_postevent(cdk_poller_t* poller, void (*cb)(void*), void* arg, bool totail);
extern void cdk_net_close(cdk_channel_t* channel);
extern void cdk_net_exit(void);
extern void cdk_net_startup(int nthrds);
extern void cdk_net_cleanup(void);
/** sync */
extern void cdk_net_startup2(void);
extern void cdk_net_cleanup2(void);
extern cdk_sock_t cdk_net_listen2(const char* protocol, const char* host, const char* port);
extern cdk_sock_t cdk_net_accept2(cdk_sock_t sock);
extern cdk_sock_t cdk_net_dial2(const char* protocol, const char* host, const char* port);
extern ssize_t cdk_net_recv2(cdk_sock_t sock, void* buf, int size);
extern ssize_t cdk_net_send2(cdk_sock_t sock, void* buf, int size);
extern ssize_t cdk_net_recvall2(cdk_sock_t sock, void* buf, int size);
extern ssize_t cdk_net_sendall2(cdk_sock_t sock, void* buf, int size);
extern ssize_t cdk_net_recvfrom2(cdk_sock_t sock, void* buf, int size, struct sockaddr_storage* ss, socklen_t* sslen);
extern ssize_t cdk_net_sendto2(cdk_sock_t sock, void* buf, int size, struct sockaddr_storage* ss, socklen_t sslen);
extern void cdk_net_close2(cdk_sock_t sock);
extern void cdk_net_recvtimeo2(cdk_sock_t sock, int timeout_ms);
extern void cdk_net_sendtimeo2(cdk_sock_t sock, int timeout_ms);