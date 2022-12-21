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

#include "unix-netpoller.h"
#include "cdk/cdk-net.h"
#include "cdk/cdk-memory.h"
#include "cdk/cdk-logger.h"
#include <stdlib.h>

#if defined(__linux__)

#define MAX_EPOLL_EVENTS 1024

#include <sys/epoll.h>
#include <errno.h>

static int epfd;

void _netpoller_create(void) {

	if (!epfd) {
		epfd = epoll_create1(0);
	}
	return;
}

void _netpoller_destroy(void) {

	if (epfd) {
		cdk_net_close(epfd);
	}
	return;
}

void _netpoller_register(sock_t s, netpoller_cmd_t c, netpoller_handler_t* h) {

	netpoller_ctx_t* ctx = cdk_malloc(sizeof(netpoller_ctx_t));

	ctx->cmd    = c;
	ctx->fd     = s;
	ctx->h      = h;

	struct epoll_event ee;
	memset(&ee, 0, sizeof(struct epoll_event));

	if (c & _NETPOLLER_CMD_A) {
		ee.events = EPOLLIN | EPOLLET;
	}
	if (c & _NETPOLLER_CMD_C) {
		ee.events = EPOLLOUT | EPOLLET;
	}
	if (c & _NETPOLLER_CMD_R) {
		ee.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
	}
	if (c & _NETPOLLER_CMD_W) {
		ee.events = EPOLLOUT | EPOLLET | EPOLLONESHOT;
	}
	ee.data.ptr = ctx;

	epoll_ctl(epfd, EPOLL_CTL_ADD, s, (struct epoll_event*)&ee);
}

void _netpoller_unregister(sock_t s) {

	epoll_ctl(epfd, EPOLL_CTL_DEL, s, NULL);
}

void _netpoller_poll(void) {

	struct epoll_event events[MAX_EPOLL_EVENTS];

	while (true) {
		int r;
		do {
			r = epoll_wait(epfd, (struct epoll_event*)&events, MAX_EPOLL_EVENTS, -1);
		} while (r == -1 && errno == EINTR);
		if (r < 0) {
			abort();
		}
		for (int i = 0; i < r; i++) {
			netpoller_ctx_t* ctx = events[i].data.ptr;

			if (events[i].events & EPOLLIN) {
				if (ctx->cmd & _NETPOLLER_CMD_A) {

				}
				if (ctx->cmd & _NETPOLLER_CMD_R) {

				}
			}
			if (events[i].events & EPOLLOUT) {
				if (ctx->cmd & _NETPOLLER_CMD_C) {

				}
				if (ctx->cmd & _NETPOLLER_CMD_W) {

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