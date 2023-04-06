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
#include "net/cdk-net-connection.h"
#include "cdk/deprecated/c11-threads.h"
#include "cdk/container/cdk-list.h"
#include "wepoll/wepoll.h"

static atomic_flag once_create = ATOMIC_FLAG_INIT;
static atomic_flag once_destroy = ATOMIC_FLAG_INIT;
static cdk_poller_t* pollers;
static atomic_size_t nslaves;
static atomic_size_t idx;

cdk_poller_t* platform_poller_retrieve(bool master)
{
    if (master) {
        return &(pollers[0]);
    }
    if (atomic_load(&idx) == atomic_load(&nslaves)) {
        atomic_store(&idx, 0);
    }
    return (pollers + (atomic_fetch_add(&idx, 1) % atomic_load(&nslaves)));
}

int platform_poller_poll(void* arg) {
    cdk_pollevent_t events[MAX_PROCESS_EVENTS];

    cdk_poller_t* poller = arg;
    poller->tid = thrd_current();

    while (true)
    {
        int nevents = platform_event_wait(poller->pfd, events);
        for (int i = 0; i < nevents; i++)
        {
            cdk_net_conn_t* conn = events[i].ptr;
            cdk_net_connection_process(conn);
        }
    }
    return 0;
}

void platform_poller_create(void)
{
    if (atomic_flag_test_and_set(&once_create)) {
        return;
    }
    pollers = malloc(sizeof(cdk_poller_t) * (atomic_load(&nslaves) + 1));
    if (pollers) {
        memset(pollers, 0, sizeof(cdk_poller_t) * (atomic_load(&nslaves) + 1));
        pollers[0].pfd = epoll_create1(0);
        
        for (int i = 1; i < (atomic_load(&nslaves) + 1); i++)
        {
            thrd_t tid;
            pollers[i].pfd = epoll_create1(0);
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
    }
    free(pollers);
    pollers = NULL;
}