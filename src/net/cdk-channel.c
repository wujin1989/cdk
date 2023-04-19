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

#include "platform/platform-socket.h"
#include "platform/platform-event.h"
#include "platform/platform-channel.h"
#include "cdk/container/cdk-list.h"
#include "cdk-channel.h"
#include "cdk/cdk-timer.h"
#include "cdk/net/cdk-net.h"
#include "cdk/container/cdk-rbtree.h"

extern cdk_timer_t timer;

static void __channel_handle_accept(cdk_channel_t* channel) {
    platform_channel_accept(channel);
}

static void __channel_handle_recv(cdk_channel_t* channel) {
    platform_channel_recv(channel);
}

static void __channel_handle_send(cdk_channel_t* channel) {
    platform_channel_send(channel);
}

static void __channel_handle_connect(cdk_channel_t* channel) {
    platform_channel_connect(channel);
}

static void __channel_destroy_callback(void* param) {
    cdk_channel_t* channel = param;

    mtx_destroy(&channel->mtx);
    free(channel);
    channel = NULL;
}

cdk_channel_t* cdk_channel_create(cdk_poller_t* poller, cdk_sock_t sock, int cmd, cdk_handler_t* handler)
{
    cdk_channel_t* channel = malloc(sizeof(cdk_channel_t));

    if (channel) {
        memset(channel, 0, sizeof(cdk_channel_t));
        channel->poller = poller;
        channel->cmd = cmd;
        channel->fd = sock;
        channel->handler = handler;
        channel->type = platform_socket_socktype(sock);
        channel->active = true;
        mtx_init(&channel->mtx, mtx_plain);

        if (channel->type == SOCK_STREAM) {
            channel->tcp.connected = false;
            channel->tcp.rxbuf.len = MAX_IOBUF_SIZE;
            channel->tcp.rxbuf.off = 0;
            channel->tcp.rxbuf.buf = malloc(MAX_IOBUF_SIZE);
            if (channel->tcp.rxbuf.buf) {
                memset(channel->tcp.rxbuf.buf, 0, MAX_IOBUF_SIZE);
            }
            cdk_list_init(&(channel->tcp.txlist));
        }
        if (channel->type == SOCK_DGRAM) {
            channel->udp.rxbuf.len = MAX_IOBUF_SIZE;
            channel->udp.rxbuf.off = 0;
            channel->udp.rxbuf.buf = malloc(MAX_IOBUF_SIZE);
            if (channel->udp.rxbuf.buf) {
                memset(channel->udp.rxbuf.buf, 0, MAX_IOBUF_SIZE);
            }
            cdk_list_init(&(channel->udp.txlist));
        }
        platform_event_add(poller->pfd, channel->fd, cmd, channel);
        return channel;
    }
    return NULL;
}

void cdk_channel_modify(cdk_channel_t* channel) {
    platform_event_mod(channel->poller->pfd, channel->fd, channel->cmd, channel);
}

void cdk_channel_destroy(cdk_channel_t* channel)
{
    mtx_lock(&channel->mtx);
    channel->active = false;

    platform_event_del(channel->poller->pfd, channel->fd);
    platform_socket_close(channel->fd);

    if (channel->type == SOCK_STREAM) {
        channel->tcp.connected = false;

        free(channel->tcp.rxbuf.buf);
        channel->tcp.rxbuf.buf = NULL;

        while (!cdk_list_empty(&(channel->tcp.txlist))) {
            cdk_txlist_node_t* e = cdk_list_data(cdk_list_head(&(channel->tcp.txlist)), cdk_txlist_node_t, n);
            cdk_list_remove(&(e->n));
            free(e);
            e = NULL;
        }
    }
    if (channel->type == SOCK_DGRAM) {
        free(channel->udp.rxbuf.buf);
        channel->udp.rxbuf.buf = NULL;

        while (!cdk_list_empty(&(channel->udp.txlist))) {
            cdk_txlist_node_t* e = cdk_list_data(cdk_list_head(&(channel->udp.txlist)), cdk_txlist_node_t, n);
            cdk_list_remove(&(e->n));
            free(e);
            e = NULL;
        }
    }
    mtx_unlock(&channel->mtx);

    cdk_timer_add(&timer, __channel_destroy_callback, channel, 10000, false);
}

void cdk_channel_process(cdk_channel_t* channel)
{
    switch (channel->cmd)
    {
    case PLATFORM_EVENT_A:
        __channel_handle_accept(channel);
        break;
    case PLATFORM_EVENT_R:
        __channel_handle_recv(channel);
        break;
    case PLATFORM_EVENT_C:
        __channel_handle_connect(channel);
        break;
    case PLATFORM_EVENT_W:
        __channel_handle_send(channel);
        break;
    default:
        break;
    }
}