/** Copyright (c) 2022, Wu Jin <wujin.developer@gmail.com>
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

#include "unix-event.h"

#define _EVENT_R    0x1
#define _EVENT_W    0x2
#define _EVENT_A    0x4
#define _EVENT_C    0x8

void _event_add(int pfd, sock_t sfd, int event, void* ud) {

	struct epoll_event ee;
	memset(&ee, 0, sizeof(struct epoll_event));

	if (event & _EVENT_A) {
		ee.events |= EPOLLIN;
	}
	if (event & _EVENT_C) {
		ee.events |= EPOLLOUT;
	}
	if (event & _EVENT_R) {
		ee.events |= EPOLLIN;
	}
	if (event & _EVENT_W) {
		ee.events |= EPOLLOUT;
	}
	ee.data.ptr = ud;

	epoll_ctl(pfd, EPOLL_CTL_ADD, sfd, (struct epoll_event*)&ee);
}

void _event_mod(int pfd, sock_t sfd, int event, void* ud) {

	struct epoll_event ee;
	memset(&ee, 0, sizeof(struct epoll_event));

	if (event & _EVENT_A) {
		ee.events |= EPOLLIN;
	}
	if (event & _EVENT_C) {
		ee.events |= EPOLLOUT;
	}
	if (event & _EVENT_R) {
		ee.events |= EPOLLIN;
	}
	if (event & _EVENT_W) {
		ee.events |= EPOLLOUT;
	}
	ee.data.ptr = ud;

	epoll_ctl(pfd, EPOLL_CTL_MOD, sfd, (struct epoll_event*)&ee);
}

void _event_del(int pfd, sock_t sfd) {

	epoll_ctl(pfd, EPOLL_CTL_DEL, sfd, NULL);
}