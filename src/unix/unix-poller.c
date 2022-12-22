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

#include "unix-poller.h"
#include "unix-net.h"
#include "cdk/cdk-net.h"
#include "cdk/cdk-memory.h"
#include <stdlib.h>
#include <string.h>

#if defined(__linux__)

#define MAX_EPOLL_EVENTS 1024

#include <sys/epoll.h>
#include <errno.h>

static int epfd;

void _poller_create(void) {

	if (!epfd) {
		epfd = epoll_create1(0);
	}
	return;
}

void _poller_destroy(void) {

	if (epfd) {
		cdk_net_close(epfd);
	}
	return;
}

void _poller_register(sock_t s, poller_cmd_t c, poller_handler_t* h) {

	poller_conn_t* conn = cdk_malloc(sizeof(poller_conn_t));

	conn->cmd    = c;
	conn->fd     = s;
	conn->h      = h;

	struct epoll_event ee;
	memset(&ee, 0, sizeof(struct epoll_event));

	if (c & _POLLER_CMD_A) {
		ee.events = EPOLLIN | EPOLLET;
	}
	if (c & _POLLER_CMD_C) {
		ee.events = EPOLLOUT | EPOLLET;
	}
	if (c & _POLLER_CMD_R) {
		ee.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
	}
	if (c & _POLLER_CMD_W) {
		ee.events = EPOLLOUT | EPOLLET | EPOLLONESHOT;
	}
	ee.data.ptr = conn;

	epoll_ctl(epfd, EPOLL_CTL_ADD, s, (struct epoll_event*)&ee);
}

void _poller_unregister(sock_t s) {

	epoll_ctl(epfd, EPOLL_CTL_DEL, s, NULL);
}

void _poller_poll(void) {

	struct epoll_event events[MAX_EPOLL_EVENTS] = {0, {0}};

	while (true) {
		int r;
		do {
			r = epoll_wait(epfd, (struct epoll_event*)&events, MAX_EPOLL_EVENTS, -1);
		} while (r == -1 && errno == EINTR);
		if (r < 0) {
			abort();
		}
		for (int i = 0; i < r; i++) {
			poller_conn_t* conn = events[i].data.ptr;

			if (events[i].events & EPOLLIN) {
				if (conn->cmd & _POLLER_CMD_A) {
					sock_t c;

					while ((c = _tcp_accept(conn->fd)) > 0) {
						_poller_register(c, _POLLER_CMD_R, conn->h);
						conn->h->on_accept(c);
					}
				}
				if (conn->cmd & _POLLER_CMD_R) {
					char buf[8192];
					ssize_t n;
					while (n = _net_recv(conn->fd, buf, sizeof(buf))) {
						conn->h->on_read(conn->fd, buf, n);
					}
					if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
						continue;
					}
					/**
					 *  (n < 0 && errno != EAGAIN || errno != EWOULDBLOCK) or n == 0;
					 *  thus close it.
					 */
					_poller_unregister(conn->fd);
					_net_close(conn->fd);
				}
			}
			if (events[i].events & EPOLLOUT) {
				if (conn->cmd & _POLLER_CMD_C) {

				}
				if (conn->cmd & _POLLER_CMD_W) {

				}
			}
			if (events[i].events & EPOLLERR) {

			}
			if (events[i].events & EPOLLHUP) {

			}
		}
	}
}
#endif