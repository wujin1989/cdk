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
#include "cdk/container/cdk-list.h"
#include "net/cdk-channel.h"

extern void _eventfd_read(cdk_channel_t* channel, void* buf, size_t len);
extern void _eventfd_close(cdk_channel_t* channel, char* error);

void platform_poller_poll(cdk_poller_t* poller) {
    cdk_pollevent_t events[MAX_PROCESS_EVENTS];

    cdk_handler_t handler = {
        .on_read = _eventfd_read,
        .on_close = _eventfd_close
    };
    cdk_unpack_t unpacker = {
        .type = UNPACK_TYPE_FIXEDLEN,
        .fixedlen.len = sizeof(int)
    };
    cdk_channel_t* channel = cdk_channel_create(poller, poller->evfds[1], &handler);
    if (channel) {
        channel->tcp.tls = NULL;
        memcpy(&channel->tcp.unpacker, &unpacker, sizeof(cdk_unpack_t));

        platform_event_add(channel->poller->pfd, channel->fd, EVENT_TYPE_R, channel);
        channel->events |= EVENT_TYPE_R;
    }
    while (poller->active)
    {
        int nevents = platform_event_wait(poller->pfd, events);
        for (int i = 0; i < nevents; i++)
        {
            cdk_channel_t* channel = events[i].ptr;
            uint32_t tevents = events[i].events;

            if ((channel->events & EVENT_TYPE_A) && (tevents & EVENT_TYPE_R)) {
                cdk_channel_accept(channel);
            }
            if ((channel->events & EVENT_TYPE_R) && (tevents & EVENT_TYPE_R)) {
                cdk_channel_recv(channel);
            }
            if ((channel->events & EVENT_TYPE_C) && (tevents & EVENT_TYPE_W)) {
                cdk_channel_connect(channel);
            }
            if ((channel->events & EVENT_TYPE_W) && (tevents & EVENT_TYPE_W)) {
                cdk_channel_send(channel);
            }
        }
    }
}

cdk_poller_t* platform_poller_create(void)
{
    cdk_poller_t* poller = malloc(sizeof(cdk_poller_t));

    if (poller) {
        poller->pfd = platform_socket_pollfd_create();
        poller->tid = thrd_current();
        poller->active = true;

        cdk_list_init(&poller->evlist);
        mtx_init(&poller->evmtx, mtx_plain);
        platform_socket_socketpair(AF_INET, SOCK_STREAM, 0, poller->evfds);
    }
    return poller;
}

void platform_poller_destroy(cdk_poller_t* poller)
{
    poller->active = false;
    platform_socket_pollfd_destroy(poller->pfd);

    platform_socket_close(poller->evfds[0]);
    platform_socket_close(poller->evfds[1]);

    while (!cdk_list_empty(&poller->evlist))
    {
        cdk_event_t* e = cdk_list_data(cdk_list_head(&poller->evlist), cdk_event_t, node);
        cdk_list_remove(&e->node);

        free(e);
        e = NULL;
    }
    mtx_destroy(&poller->evmtx);

    free(poller);
    poller = NULL;
}