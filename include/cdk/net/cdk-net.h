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

extern void           cdk_net_ntop(struct sockaddr_storage* ss, cdk_addrinfo_t* ai);
extern void           cdk_net_pton(cdk_addrinfo_t* ai, struct sockaddr_storage* ss);
extern void           cdk_net_obtain_addr(cdk_sock_t sock, cdk_addrinfo_t* ai, bool peer);
extern int            cdk_net_af(cdk_sock_t sock);
extern int            cdk_net_socktype(cdk_sock_t sock);
extern void           cdk_net_set_recvbuf(cdk_sock_t sock, int val);
extern void           cdk_net_set_sendbuf(cdk_sock_t sock, int val);
extern cdk_channel_t* cdk_net_listen(const char* type, const char* host, const char* port, cdk_handler_t* handler);
extern cdk_channel_t* cdk_net_dial(const char* type, const char* host, const char* port, cdk_handler_t* handler);
extern void           cdk_net_poll(void);
extern void           cdk_net_postrecv(cdk_channel_t* channel);
extern void           cdk_net_postsend(cdk_channel_t* channel, void* data, size_t size);
extern void           cdk_net_postevent(cdk_poller_t* poller, cdk_event_t* event);
extern void           cdk_net_close(cdk_channel_t* channel);
extern void           cdk_net_unpacker_init(cdk_channel_t* channel, cdk_unpack_t* unpacker);
extern void           cdk_net_startup(int nworkers, cdk_tlsconf_t* tlsconf);
extern void           cdk_net_cleanup(void);
