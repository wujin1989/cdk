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
#include "cdk/cdk-timer.h"
#include "cdk/cdk-time.h"
#include "cdk/net/cdk-net.h"
#include "cdk/container/cdk-heap.h"
#include "cdk/container/cdk-rbtree.h"
#include "net/channel.h"
#include "net/txlist.h"
#include <limits.h>

static inline void _event_handle(cdk_poller_t* poller) {
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

static void _channel_handle(cdk_channel_t* channel, uint32_t mask) {
    if (mask & EVENT_RD) {
        if (channel->type == SOCK_STREAM) {
            if (channel->tcp.accepting) {
                channel_accept(channel);
            } else {
                channel_recv(channel);
            }
        }
        if (channel->type == SOCK_DGRAM) {
            if (channel->udp.sslmap) {
                if (channel->udp.accepting) {
                    channel->udp.peer.sslen = sizeof(struct sockaddr_storage);
                    recvfrom(channel->fd,
                        channel->rxbuf.buf,
                        DEFAULT_IOBUF_SIZE, MSG_PEEK,
                        (struct sockaddr*)&channel->udp.peer.ss,
                        &channel->udp.peer.sslen);

                    cdk_address_t addrinfo = { 0 };
                    cdk_net_ntop(&channel->udp.peer.ss, &addrinfo);
                    sprintf(channel->udp.peer.human, "%s:%d", addrinfo.addr, addrinfo.port);

                    cdk_rbtree_key_t key = { .str = channel->udp.peer.human };
                    cdk_rbtree_node_t* n = cdk_rbtree_find(channel->udp.sslmap, key);

                    if (!n) {
                        channel_accept(channel);
                    } else {
                        dtls_sslmap_entry_t* entry = cdk_rbtree_data(n, dtls_sslmap_entry_t, node);
                        channel->udp.ssl = entry->ssl;
                        channel_recv(channel);
                    }
                } else {
                    channel_recv(channel);
                }
            } else {
                channel_recv(channel);
            }
        }
    }
    if (mask & EVENT_WR) {
        if (channel->type == SOCK_STREAM) {
            if (channel->tcp.connecting) {
                channel_connect(channel);
            } else {
                channel_send(channel);
            }
        }
        if (channel->type == SOCK_DGRAM) {
            channel_send(channel);
        }
    }
}

static inline int _timeout_update(cdk_poller_t* poller) {
    if (cdk_heap_empty(&poller->timermgr.heap)) {
        return INT_MAX - 1;
    }
    uint64_t now = cdk_time_now();
    cdk_timer_t* timer = cdk_heap_data(cdk_heap_min(&poller->timermgr.heap), cdk_timer_t, node);
    if ((timer->birth + timer->expire) <= now) {
        return 0;
    } else {
        return (timer->birth + timer->expire) - now;
    }
}

static inline void _timer_handle(cdk_poller_t* poller) {
    do {
        cdk_timer_t* timer = cdk_heap_data(cdk_heap_min(&poller->timermgr.heap), cdk_timer_t, node);
        timer->routine(timer->param);
        if (timer->repeat) {
            cdk_timer_reset(&poller->timermgr, timer, timer->expire);
        } else {
            cdk_timer_del(&poller->timermgr, timer);
        }
    } while (!_timeout_update(poller));
}

void poller_poll(cdk_poller_t* poller) {
    cdk_pollevent_t events[MAX_PROCESS_EVENTS] = {0};
	while (poller->active) {
		int nevents = platform_event_wait(poller->pfd, events, _timeout_update(poller));
		for (int i = 0; i < nevents; i++) {
			void* ud = events[i].ptr;
			uint32_t mask = events[i].events;
			if (!ud) {
				abort();
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

        cdk_list_init(&poller->evlist);
        cdk_list_init(&poller->chlist);
        cdk_timer_manager_init(&poller->timermgr);

        mtx_init(&poller->evmtx, mtx_plain);
        platform_socket_socketpair(AF_INET, SOCK_STREAM, 0, poller->evfds);
        platform_socket_nonblock(poller->evfds[1]);
        platform_event_add(poller->pfd, poller->evfds[1], EVENT_RD, &poller->evfds[1]);
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