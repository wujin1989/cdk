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
#include "net/channel.h"

static inline void _eventfd_read(cdk_channel_t* channel, void* buf, size_t len) {
    mtx_lock(&channel->poller->evmtx);
    cdk_event_t* e = NULL;
    if (!cdk_list_empty(&channel->poller->evlist)) {
        e = cdk_list_data(cdk_list_head(&channel->poller->evlist), cdk_event_t, node);
        cdk_list_remove(&e->node);
    }
    mtx_unlock(&channel->poller->evmtx);
    if (e) {
        e->cb(e->arg);
        free(e);
        e = NULL;
    }
}

static inline void _eventfd_close(cdk_channel_t* channel, const char* error) {
}

static cdk_unpack_t eventfd_unpacker = {
    .type = UNPACK_TYPE_FIXEDLEN,
    .fixedlen.len = sizeof(int)
};

static cdk_handler_t eventfd_handler = {
    .on_read = _eventfd_read,
    .on_close = _eventfd_close,
    .unpacker = &eventfd_unpacker
};

void poller_poll(cdk_poller_t* poller) {
    cdk_pollevent_t events[MAX_PROCESS_EVENTS];
    while (poller->active) {
        int nevents = platform_event_wait(poller->pfd, events);
        for (int i = 0; i < nevents; i++) {
            cdk_channel_t* channel = events[i].ptr;
            uint32_t tevents = events[i].events;

            if ((channel->events & EVENT_TYPE_A) && (tevents & EVENT_TYPE_R)) {
                channel_accept(channel);
            }
            if ((channel->events & EVENT_TYPE_R) && (tevents & EVENT_TYPE_R)) {
                channel_recv(channel);
            }
            if ((channel->events & EVENT_TYPE_C) && (tevents & EVENT_TYPE_W)) {
                channel_connect(channel);
            }
            if ((channel->events & EVENT_TYPE_W) && (tevents & EVENT_TYPE_W)) {
                channel_send(channel);
            }
        }
    }
}

cdk_poller_t* poller_create(void) {
    cdk_poller_t* poller = malloc(sizeof(cdk_poller_t));

    if (poller) {
        poller->pfd = platform_socket_pollfd_create();
        poller->tid = thrd_current();
        poller->active = true;

        cdk_list_init(&poller->evlist);
        mtx_init(&poller->evmtx, mtx_plain);
        platform_socket_socketpair(AF_INET, SOCK_STREAM, 0, poller->evfds);

        poller->wakeup = channel_create(poller, poller->evfds[1], &eventfd_handler);
        if (poller->wakeup) {
            if (!channel_is_reading(poller->wakeup)) {
                channel_enable_read(poller->wakeup);
            }
        }
    }
    return poller;
}

void poller_destroy(cdk_poller_t* poller) {
    poller->active = false;
    platform_socket_pollfd_destroy(poller->pfd);

    platform_socket_close(poller->evfds[0]);
    platform_socket_close(poller->evfds[1]);

    while (!cdk_list_empty(&poller->evlist)) {
        cdk_event_t* e = cdk_list_data(cdk_list_head(&poller->evlist), cdk_event_t, node);
        cdk_list_remove(&e->node);

        free(e);
        e = NULL;
    }
    mtx_destroy(&poller->evmtx);

    free(poller->wakeup);
    poller->wakeup = NULL;

    free(poller);
    poller = NULL;
}