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
#include "net/cdk-connection.h"
#include "cdk/cdk-timer.h"
#include "cdk/cdk-utils.h"
#include "cdk/container/cdk-rbtree.h"
#include "wepoll/wepoll.h"

static atomic_flag once_create = ATOMIC_FLAG_INIT;
static atomic_flag once_destroy = ATOMIC_FLAG_INIT;
static cdk_poller_t* pollers;
static atomic_size_t nslaves;
static atomic_size_t idx;
extern cdk_timer_t timer;

static void __poller_init(cdk_poller_t* poller) {
    poller->pfd = epoll_create1(0);
    platform_socket_socketpair(AF_INET, SOCK_STREAM, 0, poller->evfds);
    mtx_init(&poller->evmtx, mtx_plain);
    cdk_list_init(&poller->evlist);
}

static void __handle_read(cdk_net_conn_t* conn, void* buf, size_t len)
{
    mtx_lock(&conn->poller->evmtx);
    if (!cdk_list_empty(&conn->poller->evlist))
    {
        cdk_event_t* e = cdk_list_data(cdk_list_head(&conn->poller->evlist), cdk_event_t, node);
        cdk_list_remove(&e->node);
        e->cb(e->arg);

        free(e);
        e = NULL;
    }
    mtx_unlock(&conn->poller->evmtx);
}

static void __handle_close(cdk_net_conn_t* conn, char* error) {
    cdk_connection_destroy(conn);
}

cdk_poller_t* platform_poller_retrieve(bool master)
{
    if (master) {
        return &(pollers[0]);
    }
    if (atomic_load(&idx) == atomic_load(&nslaves)) {
        atomic_store(&idx, 0);
    }
    return (pollers + 1 + (atomic_fetch_add(&idx, 1) % atomic_load(&nslaves)));
}

int platform_poller_poll(void* arg) {
    cdk_pollevent_t events[MAX_PROCESS_EVENTS];

    cdk_poller_t* poller = arg;
    poller->tid = cdk_utils_systemtid();

    cdk_net_handler_t handler = {
        .on_accept = NULL,
        .on_connect = NULL,
        .on_read = __handle_read,
        .on_write = NULL,
        .on_close = __handle_close
    };
    cdk_unpack_t unpacker = {
        .type = UNPACK_TYPE_FIXEDLEN,
        .fixedlen.len = sizeof(int)
    };
    cdk_net_conn_t* conn = cdk_connection_create(poller, poller->evfds[1], PLATFORM_EVENT_R, &handler);
    memcpy(&conn->tcp.unpacker, &unpacker, sizeof(cdk_unpack_t));

    while (true)
    {
        int nevents = platform_event_wait(poller->pfd, events);
        for (int i = 0; i < nevents; i++)
        {
            cdk_net_conn_t* conn = events[i].ptr;
            if (conn) {
                cdk_connection_process(conn);
            }
        }
    }
    return 0;
}

void platform_poller_create(void)
{
    if (atomic_flag_test_and_set(&once_create)) {
        return;
    }
    atomic_init(&idx, 0);

    pollers = malloc(sizeof(cdk_poller_t) * (atomic_load(&nslaves) + 1));
    if (pollers) {
        memset(pollers, 0, sizeof(cdk_poller_t) * (atomic_load(&nslaves) + 1));
        __poller_init(&pollers[0]);

        for (int i = 1; i < (atomic_load(&nslaves) + 1); i++)
        {
            __poller_init(&pollers[i]);
            thrd_t tid;
            thrd_create(&tid, platform_poller_poll, &(pollers[i]));
            thrd_detach(tid);
        }
    }
}

void platform_poller_destroy(void)
{
    if (atomic_flag_test_and_set(&once_destroy)) {
        return;
    }
    for (int i = 0; i < (atomic_load(&nslaves) + 1); i++) {
        epoll_close(pollers[i].pfd);
        platform_socket_close(pollers[i].evfds[0]);
        platform_socket_close(pollers[i].evfds[1]);
        mtx_destroy(&pollers[i].evmtx);
    }
    free(pollers);
    pollers = NULL;
}

void platform_poller_concurrent_slaves(int num) {
    atomic_init(&nslaves, num);
}