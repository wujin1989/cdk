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

#define DEFAULT_IOBUF_SIZE 16384

#define CHANNEL_DESTROY_REASON_USER                                            \
    "Channel destroyed due to User-triggered (normal behavior)"
#define CHANNEL_DESTROY_REASON_WR_TIMEOUT                                      \
    "Channel destroyed due to write operation timeout"
#define CHANNEL_DESTROY_REASON_RD_TIMEOUT                                      \
    "Channel destroyed due to read operation timeout"
#define CHANNEL_DESTROY_REASON_CONN_TIMEOUT                                    \
    "Channel destroyed due to connection establishment timeout"
#define CHANNEL_DESTROY_REASON_POLLER_SHUTDOWN                                 \
    "Channel destroyed due to poller shutdown"

extern cdk_channel_t* channel_create(
        cdk_poller_t*  poller,
        cdk_sock_t     sock,
        bool           udp_connected,
        cdk_handler_t* handler);
extern void channel_destroy(cdk_channel_t* channel, const char* reason);
extern void channel_recv(cdk_channel_t* channel);
extern void channel_send(cdk_channel_t* channel);
extern void
channel_send_explicit(cdk_channel_t* channel, void* data, size_t size);
extern void channel_accept(cdk_channel_t* channel);
extern void channel_connect(cdk_channel_t* channel);
extern void channel_enable_write(cdk_channel_t* channel);
extern void channel_enable_read(cdk_channel_t* channel);
extern void channel_disable_write(cdk_channel_t* channel);
extern void channel_disable_read(cdk_channel_t* channel);
extern void channel_disable_all(cdk_channel_t* channel);
extern bool channel_is_writing(cdk_channel_t* channel);
extern bool channel_is_reading(cdk_channel_t* channel);
extern void channel_tls_srv_handshake(void* param);
extern void channel_tls_cli_handshake(void* param);
extern void channel_connected(cdk_channel_t* channel);
extern void channel_connecting(cdk_channel_t* channel);
extern void channel_accepted(cdk_channel_t* channel);
extern void channel_accepting(cdk_channel_t* channel);
