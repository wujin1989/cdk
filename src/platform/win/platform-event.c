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

#include "platform/platform-event.h"
#include "wepoll/wepoll.h"

void platform_event_add(cdk_pollfd_t pfd, cdk_sock_t sfd, int events, void* ud) {
	struct epoll_event ee = {0};
	if (events & EVENT_RD) {
		ee.events |= EPOLLIN;
	}
	if (events & EVENT_WR) {
		ee.events |= EPOLLOUT;
	}
	ee.data.ptr = ud;
	epoll_ctl(pfd, EPOLL_CTL_ADD, sfd, (struct epoll_event*)&ee);
}

void platform_event_mod(cdk_pollfd_t pfd, cdk_sock_t sfd, int events, void* ud) {
	struct epoll_event ee = { 0 };
	if (events & EVENT_RD) {
		ee.events |= EPOLLIN;
	}
	if (events & EVENT_WR) {
		ee.events |= EPOLLOUT;
	}
	ee.data.ptr = ud;
	epoll_ctl(pfd, EPOLL_CTL_MOD, sfd, (struct epoll_event*)&ee);
}

void platform_event_del(cdk_pollfd_t pfd, cdk_sock_t sfd) {
	epoll_ctl(pfd, EPOLL_CTL_DEL, sfd, NULL);
}

int platform_event_wait(cdk_pollfd_t pfd, platform_pollevent_t* events, int timeout) {
	int n;
	struct epoll_event __events[MAX_PROCESS_EVENTS] = {0};
    memset(events, 0, sizeof(platform_pollevent_t) * MAX_PROCESS_EVENTS);
	do {
		n = epoll_wait(pfd, __events, MAX_PROCESS_EVENTS, timeout);
	} while (n == -1 && errno == EINTR);

	if (n < 0) {
		abort();
	}
	for (int i = 0; i < n; i++) {
		events[i].ptr = __events[i].data.ptr;
		if (__events[i].events & (EPOLLIN | EPOLLHUP | EPOLLERR)) {
			events[i].events |= EVENT_RD;
		}
		if (__events[i].events & (EPOLLOUT | EPOLLHUP | EPOLLERR)) {
			events[i].events |= EVENT_WR;
		}
	}
	return n;
}
