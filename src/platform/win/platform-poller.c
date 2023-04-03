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

#include "platform-socket.h"
#include "platform-event.h"
#include "platform-connection.h"
#include "cdk/deprecated/c11-threads.h"
#include "cdk/container/cdk-list.h"
#include "wepoll/wepoll.h"

static atomic_flag once_create = ATOMIC_FLAG_INIT;
static atomic_flag once_destroy = ATOMIC_FLAG_INIT;
static cdk_list_t pollerlst;
static atomic_int nslaves;

static cdk_poller_t* __retrive_suitable_poller(void) {

    for (cdk_list_node_t* n = cdk_list_head(&pollerlst); n != cdk_list_sentinel(&pollerlst); n = cdk_list_next(n)) {

        cdk_poller_t* poller = cdk_list_data(n, cdk_poller_t, node);
        if (thrd_equal(poller->tid, thrd_current())) {
            return poller;
        }
    }
    return NULL;
}

cdk_poller_t* platform_poller_retrive(void) {
    cdk_poller_t* poller;
    if (nslaves == 0) {
        poller = cdk_list_data(cdk_list_head(&pollerlst), cdk_poller_t, node);
    }
    else {

    }
    return poller;
}

int platform_poller_poll(void* arg) {
    cdk_pollevent_t events[MAX_PROCESS_EVENTS];

    cdk_poller_t* poller;
    poller = __retrive_suitable_poller();

    while (true)
    {
        int nevents = platform_event_wait(poller->pfd, events);
        for (int i = 0; i < nevents; i++)
        {
            cdk_net_conn_t* conn = events[i].ptr;
            platform_connection_process(conn);
        }
    }
    return 0;
}

void platform_poller_create(void)
{
    if (atomic_flag_test_and_set(&once_create)) {
        return;
    }
    platform_socket_startup();

    if (nslaves == 0) {
        cdk_poller_t* poller = malloc(sizeof(cdk_poller_t));
        if (poller) {
            poller->pfd = epoll_create1(0);
            poller->tid = thrd_current();
            cdk_list_insert_tail(&pollerlst, &poller->node);
        }
    }
    else {
        for (int i = 0; i < nslaves; i++) {
            thrd_t tid;
            thrd_create(&tid, platform_poller_poll, NULL);
            thrd_detach(tid);

            cdk_poller_t* poller = malloc(sizeof(cdk_poller_t));
            if (poller) {
                poller->pfd = epoll_create1(0);
                poller->tid = tid;
                cdk_list_insert_tail(&pollerlst, &poller->node);
            }
        }
    }
}

void platform_poller_destroy(void)
{
    if (atomic_flag_test_and_set(&once_destroy)) {
        return;
    }
    for (cdk_list_node_t* n = cdk_list_head(&pollerlst); n != cdk_list_sentinel(&pollerlst);) {

        cdk_poller_t* poller = cdk_list_data(n, cdk_poller_t, node);
        cdk_list_remove(&poller->node);
        n = cdk_list_next(&poller->node);

        free(poller);
        poller = NULL;
    }
    platform_socket_cleanup();
}