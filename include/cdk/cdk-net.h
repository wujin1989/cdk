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

#include "cdk-types.h"

extern void               cdk_net_rbuf(cdk_sock_t s, int v);
extern void               cdk_net_sbuf(cdk_sock_t s, int v);
extern void               cdk_net_inet_ntop(struct sockaddr_storage* ss, cdk_addrinfo_t* ai);
extern void               cdk_net_inet_pton(cdk_addrinfo_t* ai, struct sockaddr_storage* ss);
extern void               cdk_net_obtain_addr(cdk_sock_t s, cdk_addrinfo_t* ai, bool p);
extern int                cdk_net_af(cdk_sock_t s);
extern int                cdk_net_socktype(cdk_sock_t s);
extern cdk_poller_conn_t* cdk_net_listen(const char* restrict t, const char* restrict h, const char* restrict p, cdk_poller_handler_t* handler);
extern void               cdk_net_concurrent_slaves(int64_t num);
extern void               cdk_net_poll(void);
extern void               cdk_net_postsend(cdk_poller_conn_t* conn, void* data, size_t size);
extern void               cdk_net_postrecv(cdk_poller_conn_t* conn);
extern void               cdk_net_setup_splicer(cdk_poller_conn_t* conn, cdk_spliter_t* spliter);
extern void               cdk_net_close(cdk_poller_conn_t* conn);
extern cdk_poller_conn_t* cdk_net_dial(const char* restrict t, const char* restrict h, const char* restrict p, cdk_poller_handler_t* handler);
