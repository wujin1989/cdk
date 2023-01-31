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
#include "cdk/cdk-queue.h"
#include "cdk/cdk-ringbuffer.h"
#include "cdk/cdk-systeminfo.h"
#include "cdk/cdk-varint.h"
#include <stdlib.h>
#include <string.h>

#define _POLLER_CMD_R    0x1
#define _POLLER_CMD_W    0x2
#define _POLLER_CMD_A    0x4
#define _POLLER_CMD_C    0x8

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
		_net_close(epfd);
	}
	return;
}

static void __poller_builtin_splicer1(poller_conn_t* conn) {

	char* head = conn->ibuf.buf;
	char* tail = conn->ibuf.buf + conn->ibuf.off;
	char* tmp  = head;

	uint32_t accumulated = tail - head;

	while (true) {

		if (accumulated < conn->splicer.fixed.len) {
			break;
		}
		conn->h->on_read(conn, tmp, conn->splicer.fixed.len);

		tmp         += conn->splicer.fixed.len;
		accumulated -= conn->splicer.fixed.len;
	}
	if (tmp == head) {
		return;
	}
	conn->ibuf.off = accumulated;
	if (accumulated) {
		memmove(conn->ibuf.buf, tmp, accumulated);
	}
	return;
}

static void __poller_builtin_splicer2(poller_conn_t* conn) {

	char* head  = conn->ibuf.buf;
	char* tail  = conn->ibuf.buf + conn->ibuf.off;
	char* tmp   = head;

	size_t dlen = strlen(conn->splicer.textplain.delimiter);

	uint32_t accumulated = tail - head;
	if (accumulated < dlen) {
		return;
	}
	/**
	 * for performance, thus split buffer by KMP.
	 */
	int* next = cdk_malloc(dlen * sizeof(int));
	int j     = 0;

	for (int i = 1; i < dlen; i++) {

		while (j > 0 && conn->splicer.textplain.delimiter[i] != conn->splicer.textplain.delimiter[j]) {
			j = next[j - 1];
		}
		if (conn->splicer.textplain.delimiter[i] == conn->splicer.textplain.delimiter[j]) {
			j++;
		}
		next[i] = j;
	}
	j = 0;
	for (int i = 0; i < accumulated; i++) {

		while (j > 0 && tmp[i] != conn->splicer.textplain.delimiter[j]) {
			j = next[j - 1];
		}
		if (tmp[i] == conn->splicer.textplain.delimiter[j]) {
			j++;
		}
		if (j == dlen) {

			conn->h->on_read(conn, tmp, ((i - dlen + 1) + dlen));

			tmp         += (i - dlen + 1) + dlen;
			accumulated -= (i - dlen + 1) + dlen;

			i = -1;
			j = 0;
		}
	}
	cdk_free(next);
	if (tmp == head) {
		return;
	}
	conn->ibuf.off = accumulated;
	if (accumulated) {
		memmove(conn->ibuf.buf, tmp, accumulated);
	}
	return;
}

static void __poller_builtin_splicer3(poller_conn_t* conn) {

	uint32_t fs; /* frame size   */
	uint32_t hs; /* header size  */
	uint32_t ps; /* payload size */

	char* head = conn->ibuf.buf;
	char* tail = conn->ibuf.buf + conn->ibuf.off;
	char* tmp  = head;

	uint32_t accumulated = tail - head;
	
	while (true) {
		if (accumulated < conn->splicer.binary.payload) {
			break;
		}
		hs = conn->splicer.binary.payload;
		ps = 0;

		if (conn->splicer.binary.coding == LEN_FIELD_FIXEDINT) {

			ps = *((uint32_t*)(tmp + conn->splicer.binary.offset));

			if (!cdk_byteorder()) {
				ps = ntohl(ps);
			}
		}
		if (conn->splicer.binary.coding == LEN_FIELD_VARINT) {

			size_t flexible = (tail - (tmp + conn->splicer.binary.offset));

			ps = cdk_varint_decode(tmp + conn->splicer.binary.offset, &flexible);

			if (!cdk_byteorder()) {
				ps = ntohl(ps);
			}
			hs = conn->splicer.binary.payload + flexible - conn->splicer.binary.size;
		}
		fs = hs + ps + conn->splicer.binary.adj;

		if (fs > conn->ibuf.len) {
			abort();
		}
		if (accumulated < fs) {
			break;
		}
		conn->h->on_read(conn, tmp, fs);
		tmp += fs;
		accumulated -= fs;
	}
	if (tmp == head) {
		return;
	}
	conn->ibuf.off = accumulated;
	if (accumulated) {
		memmove(conn->ibuf.buf, tmp, accumulated);
	}
	return;
}

static void __poller_userdefined_splicer(poller_conn_t* conn) {

	conn->splicer.userdefined.splice(conn);
}

static void __poller_splicer(poller_conn_t* conn) {

	switch (conn->splicer.type)
	{
	case SPLICE_TYPE_FIXED: {
		__poller_builtin_splicer1(conn);
		break;
	}
	case SPLICE_TYPE_TEXTPLAIN: {
		__poller_builtin_splicer2(conn);
		break;
	}
	case SPLICE_TYPE_BINARY: {
		__poller_builtin_splicer3(conn);
		break;
	}
	case SPLICE_TYPE_USER_DEFINED: {
		__poller_userdefined_splicer(conn);
		break;
	}
	default:
		abort();
	}
	return;
}

static bool __poller_check_connect_status(poller_conn_t* conn) {

	int       e;
	socklen_t l;
	l = sizeof(int);
	getsockopt(conn->fd, SOL_SOCKET, SO_ERROR, (char*)&e, &l);
	if (e) {
		return false;
	}
	return true;
}

static bool __poller_handle_accept(poller_conn_t* conn) {

	sock_t c = _tcp_accept(conn->fd);
	if (c == -1) {
		cdk_list_remove(&conn->n);
		if ((errno != EAGAIN || errno != EWOULDBLOCK)) {
			conn->h->on_close(conn);
		}
		if ((errno == EAGAIN || errno == EWOULDBLOCK)) {
			_poller_post_accept(conn);
		}
		return false;
	}
	poller_conn_t* nconn = _poller_conn_create(c, _POLLER_CMD_R, conn->h);
	conn->h->on_accept(nconn);
	
	return true;
}

static bool __poller_handle_connect(poller_conn_t* conn) {

	cdk_list_remove(&conn->n);
	if (__poller_check_connect_status(conn)) {
		conn->h->on_connect(conn);
	}
	else {
		conn->h->on_close(conn);
	}
	return false;
}

static bool __poller_handle_recv(poller_conn_t* conn) {

	ssize_t n = _net_recv(conn->fd, conn->ibuf.buf + conn->ibuf.off, MAX_IOBUF_SIZE);
	if (n == -1) {
		cdk_list_remove(&conn->n);
		if ((errno != EAGAIN || errno != EWOULDBLOCK)) {
			conn->h->on_close(conn);
		}
		if ((errno == EAGAIN || errno == EWOULDBLOCK)) {
			_poller_post_recv(conn);
		}
		return false;
	}
	if (n == 0) {
		cdk_list_remove(&conn->n);
		conn->h->on_close(conn);
		return false;
	}
	conn->ibuf.off += n;
	__poller_splicer(conn);

	return true;
}

static bool __poller_handle_send(poller_conn_t* conn) {

	while (conn->obuf.off < conn->obuf.len) {

		ssize_t n = _net_send(conn->fd, conn->obuf.buf + conn->obuf.off, conn->obuf.len - conn->obuf.off);
		if (n == -1) {
			cdk_list_remove(&conn->n);
			if ((errno != EAGAIN || errno != EWOULDBLOCK)) {
				conn->h->on_close(conn);
			}
			if ((errno == EAGAIN || errno == EWOULDBLOCK)) {
				_poller_post_send(conn);
			}
			return false;
		}
		if (n == 0) {
			cdk_list_remove(&conn->n);
			conn->h->on_close(conn);
			return false;
		}
		conn->obuf.off += n;
	}
	conn->h->on_write(conn, conn->obuf.buf, conn->obuf.len);

	return true;
}

static bool __poller_process_connection(poller_conn_t* conn, uint32_t cmd) {

	if (cmd & _POLLER_CMD_A) {

		return __poller_handle_accept(conn);
	}
	if (cmd & _POLLER_CMD_C) {

		return __poller_handle_connect(conn);
	}
	if (cmd & _POLLER_CMD_R) {

		return __poller_handle_recv(conn);
	}
	if (cmd & _POLLER_CMD_W) {

		return __poller_handle_send(conn);
	}
	return false;
}

int _poller_worker(void* param) {

	_poller_poll();
	return 0;
}

void _poller_setup_splicer(poller_conn_t* conn, splicer_profile_t* splicer) {

	memcpy(&conn->splicer, splicer, sizeof(splicer_profile_t));
}

poller_conn_t* _poller_conn_create(sock_t s, uint32_t c, poller_handler_t* h) {

	poller_conn_t* conn = cdk_malloc(sizeof(poller_conn_t));

	conn->cmd = c;
	conn->fd  = s;
	conn->h   = h;

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
	ee.events |= EPOLLONESHOT;
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
	ee.events |= EPOLLONESHOT;
	ee.events |= EPOLLET;
	ee.data.ptr = conn;

	epoll_ctl(epfd, EPOLL_CTL_MOD, conn->fd, (struct epoll_event*)&ee);
}

void _poller_conn_destroy(poller_conn_t* conn) {

	epoll_ctl(epfd, EPOLL_CTL_DEL, conn->fd, NULL);
	_net_close(conn->fd);
	cdk_free(conn->ibuf.buf);
	cdk_free(conn->obuf.buf);
	cdk_free(conn);
}

void _poller_poll(void) {

	struct epoll_event events[MAX_PROCESS_EVENTS];
	list_t conns;

	cdk_list_create(&conns);
	
	while (true) {
		int r;

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

					if (!__poller_process_connection(conn, conn->cmd)) {
						break;
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

void _poller_post_accept(poller_conn_t* conn) {

	conn->cmd = _POLLER_CMD_A;
	_poller_conn_modify(conn);
}

void _poller_post_connect(poller_conn_t* conn) {

	conn->cmd = _POLLER_CMD_C;
	_poller_conn_modify(conn);
}

void _poller_post_recv(poller_conn_t* conn) {

	conn->cmd = _POLLER_CMD_R;
	_poller_conn_modify(conn);
}

void _poller_post_send(poller_conn_t* conn) {

	conn->cmd = _POLLER_CMD_W;
	_poller_conn_modify(conn);
}

void _poller_send(poller_conn_t* conn, void* data, size_t size) {
	
	while (conn->obuf.off < conn->obuf.len) {
		
		ssize_t n = _net_send(conn->fd, conn->obuf.buf + conn->obuf.off, conn->obuf.len - conn->obuf.off);
		if (n == -1) {
			cdk_list_remove(&conn->n);
			if ((errno != EAGAIN || errno != EWOULDBLOCK)) {
				conn->h->on_close(conn);
			}
			if ((errno == EAGAIN || errno == EWOULDBLOCK)) {
				_poller_post_send(conn);
			}
			return;
		}
		if (n == 0) {
			cdk_list_remove(&conn->n);
			conn->h->on_close(conn);
			return;
		}
		conn->obuf.off += n;
	}
	conn->h->on_write(conn, data, size);
}

#endif