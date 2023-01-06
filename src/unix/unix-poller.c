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
#include "cdk/cdk-time.h"
#include "cdk/cdk-list.h"
#include <stdlib.h>
#include <string.h>

#define _POLLER_CMD_R    0x1
#define _POLLER_CMD_W    0x2
#define _POLLER_CMD_A    0x4
#define _POLLER_CMD_C    0x8

#define MAX_IOBUF_SIZE     (16 * 1024)
#define MAX_PROCESS_EVENTS 1024
#define YIELDTIME          10

#if defined(__linux__)

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

poller_conn_t* _poller_conn_create(sock_t s, uint32_t c, poller_handler_t* h) {

	poller_conn_t* conn = cdk_malloc(sizeof(poller_conn_t));

	conn->cmd    = c;
	conn->fd     = s;
	conn->h      = h;
	
	memset(&conn->iobuf, 0, sizeof(conn_buf_t));

	cdk_list_init_node(&conn->n);

	struct epoll_event ee;
	memset(&ee, 0, sizeof(struct epoll_event));

	if (c & _POLLER_CMD_A) {
		ee.events |= EPOLLIN;
	}
	if (c & _POLLER_CMD_C) {
		ee.events |= EPOLLOUT;
	}
	if (c & _POLLER_CMD_R) {
		ee.events |= EPOLLIN;
	}
	if (c & _POLLER_CMD_W) {
		ee.events |= EPOLLOUT;
	}
	if ((c & _POLLER_CMD_R) || (c & _POLLER_CMD_W)) {
		ee.events |= EPOLLONESHOT;
	}
	ee.events |= EPOLLET;
	ee.data.ptr = conn;

	epoll_ctl(epfd, EPOLL_CTL_ADD, s, (struct epoll_event*)&ee);

	return conn;
}

void _poller_conn_modify(poller_conn_t* conn) {

	struct epoll_event ee;
	memset(&ee, 0, sizeof(struct epoll_event));

	if (conn->cmd & _POLLER_CMD_A) {
		ee.events |= EPOLLIN;
	}
	if (conn->cmd & _POLLER_CMD_C) {
		ee.events |= EPOLLOUT;
	}
	if (conn->cmd & _POLLER_CMD_R) {
		ee.events |= EPOLLIN;
	}
	if (conn->cmd & _POLLER_CMD_W) {
		ee.events |= EPOLLOUT;
	}
	if ((conn->cmd & _POLLER_CMD_R) || (conn->cmd & _POLLER_CMD_W)) {
		ee.events |= EPOLLONESHOT;
	}
	ee.events |= EPOLLET;
	ee.data.ptr = conn;

	epoll_ctl(epfd, EPOLL_CTL_MOD, conn->fd, (struct epoll_event*)&ee);
}

void _poller_conn_destroy(poller_conn_t* conn) {

	epoll_ctl(epfd, EPOLL_CTL_DEL, conn->fd, NULL);
	_net_close(conn->fd);
	cdk_free(conn->iobuf.buffer.buf);
	cdk_free(conn);
}

static bool _poller_check_connect_status(poller_conn_t* conn) {
	int       e;
	socklen_t l;
	l = sizeof(int);
	getsockopt(conn->fd, SOL_SOCKET, SO_ERROR, (char*)&e, &l);
	if (e) {
		_poller_conn_destroy(conn);
		return false;
	}
	return true;
}

void _poller_poll(void) {

	struct epoll_event events[MAX_PROCESS_EVENTS];
	list_t conns;

	cdk_list_create(&conns);

	while (true) {
		int r;

		memset(events, 0, sizeof(struct epoll_event) * MAX_PROCESS_EVENTS);
		do {
			r = epoll_wait(epfd, (struct epoll_event*)&events, MAX_PROCESS_EVENTS, -1);
		} while (r == -1 && errno == EINTR);
		if (r < 0) {
			abort();
		}
		for (int i = 0; i < r; i++) {
			poller_conn_t* conn = events[i].data.ptr;
			cdk_list_insert_tail(&conns, &conn->n);
		}
		while (!cdk_list_empty(&conns)) {
			for (list_node_t* n = cdk_list_head(&conns); n != cdk_list_sentinel(&conns); ) {

				uint64_t stime, etime;
				stime = etime = cdk_timespec_get();

				poller_conn_t* conn = cdk_list_data(n, poller_conn_t, n);
				n = cdk_list_next(n);

				while ((etime - stime < YIELDTIME)) {
					if (conn->cmd & _POLLER_CMD_A) {

						sock_t c = _tcp_accept(conn->fd);
						if (c == -1) {
							cdk_list_remove(&conn->n);
							break;
						}
						poller_conn_t* nconn = _poller_conn_create(c, _POLLER_CMD_R, conn->h);
						conn->h->on_accept(nconn);
					}
					if (conn->cmd & _POLLER_CMD_R) {

						ssize_t n = _net_recv(conn->fd, conn->iobuf.buffer.buf, MAX_IOBUF_SIZE);
						if (n == -1) {
							cdk_list_remove(&conn->n);
							if ((errno != EAGAIN || errno != EWOULDBLOCK)) {
								_poller_conn_destroy(conn);
							}
							if ((errno == EAGAIN || errno == EWOULDBLOCK)) {
								_poller_post_recv(conn);
							}
							break;
						}
						if (n == 0) {
							cdk_list_remove(&conn->n);
							_poller_conn_destroy(conn);
							break;
						}
						conn->iobuf.buffer.len = n;
						conn->h->on_read(conn);
					}
					if (conn->cmd & _POLLER_CMD_C) {
						
						if (_poller_check_connect_status(conn)) {
							conn->h->on_connect(conn);
						}
						cdk_list_remove(&conn->n);
					}
					if (conn->cmd & _POLLER_CMD_W) {
						
						if (conn->iobuf.sent < conn->iobuf.buffer.len) {
							
							ssize_t n = _net_send(conn->fd, &conn->iobuf.buffer.buf[conn->iobuf.sent], conn->iobuf.buffer.len - conn->iobuf.sent);
							if (n == -1) {
								cdk_list_remove(&conn->n);
								if ((errno != EAGAIN || errno != EWOULDBLOCK)) {
									_poller_conn_destroy(conn);
								}
								if ((errno == EAGAIN || errno == EWOULDBLOCK)) {
									_poller_post_send(conn);
								}
								break;
							}
							if (n == 0) {
								cdk_list_remove(&conn->n);
								_poller_conn_destroy(conn);
								break;
							}
							conn->iobuf.sent += n;
						}
						else {
							conn->h->on_write(conn, conn->iobuf.buffer.buf, conn->iobuf.buffer.len);
						}
					}
					etime = cdk_timespec_get();
				}
			}
		}
	}
}

void _poller_listen(const char* restrict t, const char* restrict h, const char* restrict p, poller_handler_t* handler) {

	sock_t s;
	_poller_create();

	if (!strncmp(t, "tcp", strlen("tcp"))) {
		s = _net_listen(h, p, SOCK_STREAM);
	}
	if (!strncmp(t, "udp", strlen("udp"))) {
		s = _net_listen(h, p, SOCK_DGRAM);
	}
	_poller_conn_create(s, _POLLER_CMD_A, handler);
}

void _poller_dial(const char* restrict t, const char* restrict h, const char* restrict p, poller_handler_t* handler) {

	sock_t         s;
	bool           connected;
	poller_conn_t* conn;

	_poller_create();

	if (!strncmp(t, "tcp", strlen("tcp"))) {
		s = _net_dial(h, p, SOCK_STREAM, &connected);
	}
	if (!strncmp(t, "udp", strlen("udp"))) {
		s = _net_dial(h, p, SOCK_DGRAM, &connected);
	}
	conn = _poller_conn_create(s, _POLLER_CMD_C, handler);
	if (connected) {
		handler->on_connect(conn);
	}
}

void _poller_post_recv(poller_conn_t* conn) {

	if (conn->iobuf.buffer.buf == NULL) {
		conn->iobuf.buffer.buf  = cdk_malloc(MAX_IOBUF_SIZE);
	}
	if (conn->iobuf.accumulated == NULL) {
		conn->iobuf.accumulated = cdk_malloc(MAX_IOBUF_SIZE);
	}
	conn->cmd = _POLLER_CMD_R;
	_poller_conn_modify(conn);
}

void _poller_post_send(poller_conn_t* conn) {

	if (conn->iobuf.buffer.buf == NULL) {
		conn->iobuf.buffer.buf  = cdk_malloc(MAX_IOBUF_SIZE);
	}
	if (conn->iobuf.accumulated == NULL) {
		conn->iobuf.accumulated = cdk_malloc(MAX_IOBUF_SIZE);
	}
	conn->cmd = _POLLER_CMD_W;
	_poller_conn_modify(conn);
}

void _poller_recv(poller_conn_t* conn, void* data, size_t size) {

	if (size > MAX_IOBUF_SIZE) {
		abort();
	}
	memset(data, 0, size);
	memcpy(data, conn->iobuf.buffer.buf, conn->iobuf.buffer.len);
	memset(conn->iobuf.buffer.buf, 0, MAX_IOBUF_SIZE);
}

void _poller_send(poller_conn_t* conn, void* data, size_t size) {

	/**
	 * the application layer guarantees read, write size.
	 */
	if (size > MAX_IOBUF_SIZE) {
		abort();
	}
	conn->iobuf.buffer.len = size;
	conn->iobuf.sent       = 0;

	memset(conn->iobuf.buffer.buf, 0, MAX_IOBUF_SIZE);
	memcpy(conn->iobuf.buffer.buf, data, size);
}

#endif