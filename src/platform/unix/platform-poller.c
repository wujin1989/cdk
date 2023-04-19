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
    cdk_channel_t* channel = cdk_channel_create(poller, poller->evfds[1], PLATFORM_EVENT_R, &handler);
    memcpy(&channel->tcp.unpacker, &unpacker, sizeof(cdk_unpack_t));

    while (poller->active)
    {
        int nevents = platform_event_wait(poller->pfd, events);
        for (int i = 0; i < nevents; i++)
        {
            cdk_channel_t* channel = events[i].ptr;
            if (channel) {
                cdk_channel_process(channel);
            }
        }
    }
}

#if defined(__linux__)
cdk_poller_t* platform_poller_create(void)
{
    cdk_poller_t* poller = malloc(sizeof(cdk_poller_t));

    if (poller) {
        poller->pfd = epoll_create1(0);
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
    close(poller->pfd);

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
#endif

#if defined(__APPLE__)
void platform_poller_create(void)
{
    
}

void platform_poller_destroy(void)
{
    
}
#endif
