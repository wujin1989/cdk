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

#if defined(__linux__)

void platform_event_add(cdk_pollfd_t pfd, cdk_sock_t sfd, int events, void* ud) {
	struct epoll_event ee;
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
	struct epoll_event ee;
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

int platform_event_wait(cdk_pollfd_t pfd, cdk_pollevent_t* events, int timeout) {
	struct epoll_event __events[MAX_PROCESS_EVENTS];
	int n;
	memset(events, 0, sizeof(cdk_pollevent_t) * MAX_PROCESS_EVENTS);
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
#endif

#if defined(__APPLE__)
void platform_event_add(cdk_pollfd_t pfd, cdk_sock_t sfd, int events, void* ud) {
	struct kevent ke;
	if (events & EVENT_RD) {
		EV_SET(&ke, sfd, EVFILT_READ, EV_ADD, 0, 0, ud);
		kevent(pfd, &ke, 1, NULL, 0, NULL);
	}
	if (events & EVENT_WR) {
		EV_SET(&ke, sfd, EVFILT_WRITE, EV_ADD, 0, 0, ud);
		kevent(pfd, &ke, 1, NULL, 0, NULL);
	}
}

void platform_event_mod(cdk_pollfd_t pfd, cdk_sock_t sfd, int events, void* ud) {
	struct kevent ke;
	if (events & EVENT_RD) {
		EV_SET(&ke, sfd, EVFILT_READ, EV_ADD, 0, 0, ud);
		kevent(pfd, &ke, 1, NULL, 0, NULL);
	}
	if (events & EVENT_WR) {
		EV_SET(&ke, sfd, EVFILT_WRITE, EV_ADD, 0, 0, ud);
		kevent(pfd, &ke, 1, NULL, 0, NULL);
	}
}

void platform_event_del(cdk_pollfd_t pfd, cdk_sock_t sfd) {
	struct kevent ke;

	EV_SET(&ke, sfd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	kevent(pfd, &ke, 1, NULL, 0, NULL);

	EV_SET(&ke, sfd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
	kevent(pfd, &ke, 1, NULL, 0, NULL);
}

int platform_event_wait(cdk_pollfd_t pfd, cdk_pollevent_t* events, int timeout) {
	struct kevent __events[MAX_PROCESS_EVENTS];

	memset(events, 0, sizeof(cdk_pollevent_t) * MAX_PROCESS_EVENTS);
	struct timespec ts = { 0, 0 };
	ts.tv_sec = (timeout / 1000UL);
	ts.tv_nsec = ((timeout % 1000UL) * 1000000UL);

	int n = kevent(pfd, NULL, 0, __events, MAX_PROCESS_EVENTS, &ts);
	/**
	 * In systems utilizing the kqueue mechanism, read and write events are handled independently, 
	 * differing from the behavior of epoll. With epoll, read and write events can be combined, 
	 * allowing the use of bitwise operations (such as the & operator) to check for specific event types on a file descriptor. 
	 * However, in kqueue, read and write events are treated as entirely separate occurrences. 
	 * This means that bitwise operations cannot be simply applied to determine the event type.
     * Consequently, when processing events returned by kqueue, each event must be examined individually to ascertain whether it is a read event or a write event, 
	 * and then the event handling logic should be updated accordingly. 
	 * This typically involves maintaining a separate state for each file descriptor to track the event types that have occurred, rather than employing bitwise operations within a single event mask. 
	 */
	for (int i = 0; i < n; i++) {
		events[i].ptr = __events[i].udata;
		if (__events[i].filter == EVFILT_READ) {
			events[i].events |= EVENT_RD;
		}
		if (__events[i].filter == EVFILT_WRITE) {
			events[i].events |= EVENT_WR;
		}
	}
	return n;
}
#endif
