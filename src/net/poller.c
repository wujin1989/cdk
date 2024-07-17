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
#include "net/txlist.h"

static inline void _eventfd_recv(cdk_poller_t* poller) {
    bool wakeup;
    platform_socket_recv(poller->evfds[1], (char*)(&wakeup), sizeof(bool));
    
    mtx_lock(&poller->evmtx);
    cdk_event_t* e = NULL;
    if (!cdk_list_empty(&poller->evlist)) {
        e = cdk_list_data(cdk_list_head(&poller->evlist), cdk_event_t, node);
        cdk_list_remove(&e->node);
    }
    mtx_unlock(&poller->evmtx);
    if (e) {
        e->cb(e->arg);
        free(e);
        e = NULL;
    }
}

static inline void _poller_event_handle(cdk_poller_t* poller) {
    _eventfd_recv(poller);
}

static inline void _poller_channel_handle(cdk_channel_t* channel, uint32_t mask) {
    if (mask & EVENT_TYPE_R) {
        (channel->type == SOCK_STREAM)
            ? ((channel->tcp.accepting)
                ? channel_accept(channel)
                : channel_recv(channel))
            : channel_recv(channel);
    }
    if (mask & EVENT_TYPE_W) {
        (channel->type == SOCK_STREAM)
            ? ((channel->tcp.connecting)
                ? channel_connect(channel)
                : channel_send(channel))
            : channel_send(channel);
    }
}

void poller_poll(cdk_poller_t* poller) {
    cdk_pollevent_t events[MAX_PROCESS_EVENTS] = {0};
	while (poller->active) {
		int nevents = platform_event_wait(poller->pfd, events);
		for (int i = 0; i < nevents; i++) {
			void* ud = events[i].ptr;
			uint32_t mask = events[i].events;
			if (!ud) {
				abort();
			}
			if (*((cdk_sock_t*)ud) == poller->evfds[1]) {
				_poller_event_handle(poller);
			}
			else {
				_poller_channel_handle((cdk_channel_t*)ud, mask);
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
        cdk_list_init(&poller->chlist);
        mtx_init(&poller->evmtx, mtx_plain);
        platform_socket_socketpair(AF_INET, SOCK_STREAM, 0, poller->evfds);
        platform_socket_nonblock(poller->evfds[1]);
        platform_event_add(poller->pfd, poller->evfds[1], EVENT_TYPE_R, &poller->evfds[1]);
    }
    return poller;
}

void poller_destroy(cdk_poller_t* poller) {
    poller->active = false;
    platform_socket_pollfd_destroy(poller->pfd);
    platform_socket_close(poller->evfds[0]);

    while (!cdk_list_empty(&poller->evlist)) {
        cdk_event_t* ev = cdk_list_data(cdk_list_head(&poller->evlist), cdk_event_t, node);
        cdk_list_remove(&ev->node);

        free(ev);
        ev = NULL;
    }
    while (!cdk_list_empty(&poller->chlist)) {
        cdk_channel_t* ch = cdk_list_data(cdk_list_head(&poller->chlist), cdk_channel_t, node);
        channel_destroy(ch, "");
    }
    mtx_destroy(&poller->evmtx);
    free(poller);
    poller = NULL;
}