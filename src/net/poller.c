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

#include "cdk/cdk-time.h"
#include "cdk/cdk-timer.h"
#include "cdk/container/cdk-heap.h"
#include "cdk/container/cdk-list.h"
#include "cdk/container/cdk-rbtree.h"
#include "cdk/net/cdk-net.h"
#include "net/channel.h"
#include "net/txlist.h"
#include "platform/platform-event.h"
#include "platform/platform-socket.h"
#include <limits.h>

static inline void _event_handle(cdk_poller_t* poller) {
    bool wakeup;
    platform_socket_recv(poller->evfds[1], (char*)(&wakeup), sizeof(bool));

    mtx_lock(&poller->evmtx);
    cdk_async_event_t* async_event = NULL;
    if (!cdk_list_empty(&poller->evlist)) {
        async_event = cdk_list_data(
            cdk_list_head(&poller->evlist), cdk_async_event_t, node);
        cdk_list_remove(&async_event->node);
    }
    mtx_unlock(&poller->evmtx);
    if (async_event) {
        async_event->task(async_event->arg);
        free(async_event);
        async_event = NULL;
    }
}

static void _channel_handle(cdk_channel_t* channel, uint32_t mask) {
    if (mask & EVENT_RD) {
        if (channel->type == SOCK_STREAM) {
            if (channel->accepting) {
                channel_accepting(channel);
            } else {
                channel_recv(channel);
            }
        } else {
            if (channel->accepting) {
                channel->accepting = false;
                channel_timers_create(channel);
                channel_recv(channel);
            } else {
                channel_recv(channel);
            }
        }
    }
    if (mask & EVENT_WR) {
        if (channel->type == SOCK_STREAM) {
            if (channel->tcp.connecting) {
                channel_connecting(channel);
            } else {
                channel_send(channel);
            }
        } else {
            channel_send(channel);
        }
    }
}

static inline int _timeout_update(cdk_poller_t* poller) {
    if (cdk_timer_empty(poller->timermgr)) {
        return INT_MAX - 1;
    }
    uint64_t     now = cdk_time_now();
    cdk_timer_t* timer = cdk_timer_min(poller->timermgr);
    if ((timer->birth + timer->expire) <= now) {
        return 0;
    } else {
        return (timer->birth + timer->expire) - now;
    }
}

static inline void _timer_handle(cdk_poller_t* poller) {
    do {
        cdk_timer_t* timer = cdk_timer_min(poller->timermgr);
        timer->routine(timer->param);
        if (timer->repeat) {
            cdk_timer_reset(poller->timermgr, timer, timer->expire);
        } else {
            cdk_timer_del(poller->timermgr, timer);
        }
    } while (!_timeout_update(poller));
}

void poller_poll(cdk_poller_t* poller) {
    platform_pollevent_t events[MAX_PROCESS_EVENTS] = {0};
    while (poller->active) {
        int nevents =
            platform_event_wait(poller->pfd, events, _timeout_update(poller));

        for (int i = 0; i < nevents; i++) {
            void*    ud = events[i].ptr;
            uint32_t mask = events[i].events;
            if (!ud) {
                return;
            }
            if (*((cdk_sock_t*)ud) == poller->evfds[1]) {
                _event_handle(poller);
            } else {
                _channel_handle((cdk_channel_t*)ud, mask);
            }
        }
        if (!_timeout_update(poller)) {
            _timer_handle(poller);
        }
    }
}

void poller_wakeup(cdk_poller_t* poller) {
    bool wakeup = true;
    platform_socket_send(poller->evfds[0], &wakeup, sizeof(bool));
}

cdk_poller_t* poller_create(void) {
    cdk_poller_t* poller = malloc(sizeof(cdk_poller_t));

    if (poller) {
        poller->pfd = platform_socket_pollfd_create();
        poller->tid = thrd_current();
        poller->active = true;
        poller->timermgr = cdk_timer_manager_create();

        cdk_list_init(&poller->evlist);
        cdk_list_init(&poller->chlist);

        mtx_init(&poller->evmtx, mtx_plain);
        platform_socket_socketpair(AF_INET, SOCK_STREAM, 0, poller->evfds);
        platform_socket_nonblock(poller->evfds[1]);
        platform_event_add(
            poller->pfd, poller->evfds[1], EVENT_RD, &poller->evfds[1]);
    }
    return poller;
}

void poller_destroy(cdk_poller_t* poller) {
    poller->active = false;
    platform_socket_pollfd_destroy(poller->pfd);
    platform_socket_close(poller->evfds[0]);

    while (!cdk_list_empty(&poller->evlist)) {
        cdk_async_event_t* async_event = cdk_list_data(
            cdk_list_head(&poller->evlist), cdk_async_event_t, node);
        cdk_list_remove(&async_event->node);

        free(async_event);
        async_event = NULL;
    }
    while (!cdk_timer_empty(poller->timermgr)) {
        cdk_timer_t* timer = cdk_timer_min(poller->timermgr);
        cdk_timer_del(poller->timermgr, timer);
    }
    while (!cdk_list_empty(&poller->chlist)) {
        cdk_channel_t* channel =
            cdk_list_data(cdk_list_head(&poller->chlist), cdk_channel_t, node);

        cdk_channel_error_t error = {
            .code = CHANNEL_ERROR_POLLER_SHUTDOWN,
            .codestr = CHANNEL_ERROR_POLLER_SHUTDOWN_STR
        };
        channel_error_update(channel, error);
        channel_destroy(channel);
    }
    mtx_destroy(&poller->evmtx);
    cdk_timer_manager_destroy(poller->timermgr);
    free(poller);
    poller = NULL;
}