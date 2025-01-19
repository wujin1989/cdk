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

extern void cdk_net_ntop(struct sockaddr_storage* ss, cdk_address_t* ai);
extern void cdk_net_pton(cdk_address_t* ai, struct sockaddr_storage* ss);
extern void cdk_net_address_make(cdk_sock_t sock, struct sockaddr_storage* ss, char* host, char* port);
extern void cdk_net_address_retrieve(cdk_sock_t sock, cdk_address_t* ai, bool peer);
extern void cdk_net_listen(const char* protocol, const char* host, const char* port, cdk_handler_t* handler, int nthrds, cdk_tls_conf_t* config);
extern void cdk_net_dial(const char* protocol, const char* host, const char* port, cdk_handler_t* handler, int nthrds, cdk_tls_conf_t* config);
extern void cdk_net_send(cdk_channel_t* channel, void* data, size_t size);
extern void cdk_net_post_task(cdk_poller_t* poller, void (*cb)(void*), void* arg, bool totail);
extern void cdk_net_close(cdk_channel_t* channel);
extern void cdk_net_exit(void);
