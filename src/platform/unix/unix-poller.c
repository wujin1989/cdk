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

#include "unix-poller.h"
#include "unix-net.h"
#include "unix-event.h"
#include "cdk/cdk-net.h"
#include "cdk/cdk-memory.h"
#include "cdk/cdk-time.h"
#include "cdk/cdk-list.h"
#include "cdk/cdk-queue.h"
#include "cdk/cdk-ringbuffer.h"
#include "cdk/cdk-systeminfo.h"
#include "cdk/cdk-varint.h"
#include "cdk/cdk-thread.h"
#include "cdk/cdk-sync.h"
#include "cdk/cdk-rbtree.h"
#include "cdk/cdk-atomic.h"
#include <stdlib.h>
#include <string.h>

enum{
	_POLLER_CMD_R,
	_POLLER_CMD_W,
	_POLLER_CMD_A,
	_POLLER_CMD_C,
};

#if defined(__linux__)

#include <sys/epoll.h>
#include <errno.h>

static mtx_t       __mutex;
static atomic_t    __nslaves     = ATOMIC_VAR_INIT(0);
static atomic_flag __once_create = ATOMIC_FLAG_INIT;
static atomic_flag __one_master  = ATOMIC_FLAG_INIT;
static int*        __slaves;
static int         __master;
static uint32_t    __idx;

typedef struct _inner_offset_buf_t {

	fifo_node_t  n;
	uint32_t     len;
	uint32_t     off;
	char         buf[];
}inner_offset_buf_t;

typedef struct _inner_owner_t {

	tid_t		id;
	rb_node_t   n;
}inner_owner_t;

static inline void __rb_insert_owner(rb_tree_t* tree, tid_t tid, rb_node_t* node) {
	
	rb_node_t** p     = &(tree->rb_root);
	rb_node_t* parent = NULL;
	
	while (*p) {
		
		parent = *p;
		inner_owner_t* e = cdk_rb_entry(parent, inner_owner_t, n);
		
		if (tid < e->id) {
			p = &(*p)->rb_left;
		}
		else if (tid > e->id) {
			p = &(*p)->rb_right;
		}
		else {
			return;
		}
	}
	cdk_rb_link_node(node, parent, p);
	cdk_rb_insert_color(tree, node);
}

static inline inner_owner_t* __rb_find_owner(rb_tree_t* tree, tid_t tid) {
	
	rb_node_t* n = tree->rb_root;
	
	while (n) {
		inner_owner_t* e = cdk_rb_entry(n, inner_owner_t, n);
		
		if (tid < e->id) {
			n = n->rb_left;
		}
		else if (tid > e->id) {
			n = n->rb_right;
		}
		else {
			return e;
		}
	}
	return NULL;
}

static void __poller_post_accept(poller_conn_t* conn) {

	conn->cmd = _EVENT_A;
	_poller_conn_modify(conn);
}

static void __poller_post_connect(poller_conn_t* conn) {

	conn->cmd = _EVENT_C;
	_poller_conn_modify(conn);
}

static void __poller_post_recv(poller_conn_t* conn) {

	conn->cmd = _EVENT_R;
	_poller_conn_modify(conn);

	return;
}

static void __poller_post_send(poller_conn_t* conn) {

	conn->cmd = _EVENT_W;
	_poller_conn_modify(conn);

	return;
}

static int __poller_loadbalance(void) {

	int pfd;

	cdk_mtx_lock(&__mutex);

	if (__idx == cdk_atomic_load(&__nslaves)) {
		__idx = 0;
	}
	pfd = *(__slaves + (__idx++ % cdk_atomic_load(&__nslaves)));

	cdk_mtx_unlock(&__mutex);

	return pfd;
}

static void __poller_builtin_splicer1(poller_conn_t* conn) {

	char* head = conn->tcp.ibuf.buf;
	char* tail = conn->tcp.ibuf.buf + conn->tcp.ibuf.off;
	char* tmp = head;

	uint32_t accumulated = tail - head;

	while (true) {

		if (accumulated < conn->tcp.splicer.fixed.len) {
			break;
		}
		conn->h->on_read(conn, tmp, conn->tcp.splicer.fixed.len);

		tmp += conn->tcp.splicer.fixed.len;
		accumulated -= conn->tcp.splicer.fixed.len;
	}
	if (tmp == head) {
		return;
	}
	conn->tcp.ibuf.off = accumulated;
	if (accumulated) {
		memmove(conn->tcp.ibuf.buf, tmp, accumulated);
	}
	return;
}

static void __poller_builtin_splicer2(poller_conn_t* conn) {

	char* head = conn->tcp.ibuf.buf;
	char* tail = conn->tcp.ibuf.buf + conn->tcp.ibuf.off;
	char* tmp = head;

	size_t dlen = strlen(conn->tcp.splicer.textplain.delimiter);

	uint32_t accumulated = tail - head;
	if (accumulated < dlen) {
		return;
	}
	/**
	 * for performance, thus split buffer by KMP.
	 */
	int* next = cdk_malloc(dlen * sizeof(int));
	int j = 0;

	for (int i = 1; i < dlen; i++) {

		while (j > 0 && conn->tcp.splicer.textplain.delimiter[i] != conn->tcp.splicer.textplain.delimiter[j]) {
			j = next[j - 1];
		}
		if (conn->tcp.splicer.textplain.delimiter[i] == conn->tcp.splicer.textplain.delimiter[j]) {
			j++;
		}
		next[i] = j;
	}
	j = 0;
	for (int i = 0; i < accumulated; i++) {

		while (j > 0 && tmp[i] != conn->tcp.splicer.textplain.delimiter[j]) {
			j = next[j - 1];
		}
		if (tmp[i] == conn->tcp.splicer.textplain.delimiter[j]) {
			j++;
		}
		if (j == dlen) {

			conn->h->on_read(conn, tmp, ((i - dlen + 1) + dlen));

			tmp += (i - dlen + 1) + dlen;
			accumulated -= (i - dlen + 1) + dlen;

			i = -1;
			j = 0;
		}
	}
	cdk_free(next);
	if (tmp == head) {
		return;
	}
	conn->tcp.ibuf.off = accumulated;
	if (accumulated) {
		memmove(conn->tcp.ibuf.buf, tmp, accumulated);
	}
	return;
}

static void __poller_builtin_splicer3(poller_conn_t* conn) {

	uint32_t fs; /* frame size   */
	uint32_t hs; /* header size  */
	uint32_t ps; /* payload size */

	char* head = conn->tcp.ibuf.buf;
	char* tail = conn->tcp.ibuf.buf + conn->tcp.ibuf.off;
	char* tmp = head;

	uint32_t accumulated = tail - head;

	while (true) {
		if (accumulated < conn->tcp.splicer.binary.payload) {
			break;
		}
		hs = conn->tcp.splicer.binary.payload;
		ps = 0;

		if (conn->tcp.splicer.binary.coding == LEN_FIELD_FIXEDINT) {

			ps = *((uint32_t*)(tmp + conn->tcp.splicer.binary.offset));
			//0 means little-endian, 1 means big-endian.
			if (!cdk_byteorder()) {
				ps = ntohl(ps);
			}
		}
		if (conn->tcp.splicer.binary.coding == LEN_FIELD_VARINT) {

			size_t flexible = (tail - (tmp + conn->tcp.splicer.binary.offset));

			ps = cdk_varint_decode(tmp + conn->tcp.splicer.binary.offset, &flexible);

			if (!cdk_byteorder()) {
				ps = ntohl(ps);
			}
			hs = conn->tcp.splicer.binary.payload + flexible - conn->tcp.splicer.binary.size;
		}
		fs = hs + ps + conn->tcp.splicer.binary.adj;

		if (fs > conn->tcp.ibuf.len) {
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
	conn->tcp.ibuf.off = accumulated;
	if (accumulated) {
		memmove(conn->tcp.ibuf.buf, tmp, accumulated);
	}
	return;
}

static void __poller_userdefined_splicer(poller_conn_t* conn) {

	conn->tcp.splicer.userdefined.splice(conn);
}

static void __poller_splicer(poller_conn_t* conn) {

	switch (conn->tcp.splicer.type)
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

static void __poller_handle_accept(poller_conn_t* conn) {

	sock_t c = _tcp_accept(conn->fd);
	if (c == -1) {
		cdk_list_remove(&conn->n);
		if ((errno != EAGAIN || errno != EWOULDBLOCK)) {
			conn->h->on_close(conn);
		}
		if ((errno == EAGAIN || errno == EWOULDBLOCK)) {
			__poller_post_accept(conn);
		}
		return;
	}
	poller_conn_t* nconn = _poller_conn_create(__poller_loadbalance(), c, _EVENT_R, conn->h);
	conn->h->on_accept(nconn);
	return;
}

static void __poller_handle_connect(poller_conn_t* conn) {

	cdk_list_remove(&conn->n);

	if (__poller_check_connect_status(conn)) {
		conn->h->on_connect(conn);
	}
	else {
		conn->h->on_close(conn);
	}
	return;
}

static void __poller_handle_recv(poller_conn_t* conn) {

	ssize_t n;

	if (conn->type == SOCK_STREAM) {
		n = _net_recv(conn->fd, conn->tcp.ibuf.buf + conn->tcp.ibuf.off, MAX_IOBUF_SIZE);

		if (n == -1) {
			cdk_list_remove(&conn->n);
			if ((errno != EAGAIN || errno != EWOULDBLOCK)) {
				conn->h->on_close(conn);
			}
			if ((errno == EAGAIN || errno == EWOULDBLOCK)) {
				__poller_post_recv(conn);
			}
			return;
		}
		if (n == 0) {
			cdk_list_remove(&conn->n);
			conn->h->on_close(conn);
			return;
		}
		conn->tcp.ibuf.off += n;
		__poller_splicer(conn);
	}
	if (conn->type == SOCK_DGRAM) {

		conn->udp.peer.sslen = sizeof(struct sockaddr_storage);
		n = _net_recvfrom(conn->fd, conn->udp.ibuf.buf, MAX_IOBUF_SIZE, &conn->udp.peer.ss, &conn->udp.peer.sslen);

		if (n == -1) {
			cdk_list_remove(&conn->n);
			if ((errno != EAGAIN || errno != EWOULDBLOCK)) {
				conn->h->on_close(conn);
			}
			if ((errno == EAGAIN || errno == EWOULDBLOCK)) {
				__poller_post_recv(conn);
			}
			return;
		}
		conn->h->on_read(conn, conn->udp.ibuf.buf, n);
	}
	return;
}

static void __poller_handle_send(poller_conn_t* conn) {

	if (conn->type == SOCK_STREAM) {

		cdk_mtx_lock(&conn->mutex);

		while (cdk_queue_empty(&(conn->tcp.olist))) {

			cdk_list_remove(&conn->n);
			cdk_mtx_unlock(&conn->mutex);
			return;
		}
		while (!cdk_queue_empty(&(conn->tcp.olist))) {

			inner_offset_buf_t* e = cdk_list_data(cdk_list_head(&(conn->tcp.olist)), inner_offset_buf_t, n);

			while (e->off < e->len) {

				ssize_t n = _net_send(conn->fd, e->buf + e->off, e->len - e->off);
				if (n == -1) {
					cdk_list_remove(&conn->n);

					if ((errno != EAGAIN || errno != EWOULDBLOCK)) {
						conn->h->on_close(conn);
					}
					if ((errno == EAGAIN || errno == EWOULDBLOCK)) {
						__poller_post_send(conn);
						cdk_mtx_unlock(&conn->mutex);
					}
					return;
				}
				if (n == 0) {
					cdk_list_remove(&conn->n);
					conn->h->on_close(conn);
					return;
				}
				e->off += n;
			}
			conn->h->on_write(conn, e->buf, e->len);

			cdk_list_remove(&(e->n));
			cdk_free(e);
		}
		cdk_mtx_unlock(&conn->mutex);
	}
	if (conn->type == SOCK_DGRAM) {
		
		cdk_mtx_lock(&conn->mutex);

		while (cdk_queue_empty(&(conn->udp.olist))) {

			cdk_list_remove(&conn->n);
			cdk_mtx_unlock(&conn->mutex);
			return;
		}
		while (!cdk_queue_empty(&(conn->udp.olist))) {
			
			inner_offset_buf_t* e = cdk_list_data(cdk_list_head(&(conn->udp.olist)), inner_offset_buf_t, n);

			ssize_t n = _net_sendto(conn->fd, e->buf, e->len, &(conn->udp.peer.ss), conn->udp.peer.sslen);
			if (n == -1) {
				cdk_list_remove(&conn->n);

				if ((errno != EAGAIN || errno != EWOULDBLOCK)) {
					conn->h->on_close(conn);
				}
				if ((errno == EAGAIN || errno == EWOULDBLOCK)) {
					__poller_post_send(conn);
					cdk_mtx_unlock(&conn->mutex);
				}
				return;
			}
			if (n == 0) {
				cdk_list_remove(&conn->n);
				conn->h->on_close(conn);
				return;
			}
			conn->h->on_write(conn, e->buf, e->len);

			cdk_list_remove(&(e->n));
			cdk_free(e);
		}
		cdk_mtx_unlock(&conn->mutex);
	}
	return;
}

static void __poller_process_connection(poller_conn_t* conn) {

	switch (conn->cmd)
	{
	case _EVENT_A:
		__poller_handle_accept(conn);
		break;
	case _EVENT_R:
		__poller_handle_recv(conn);
		break;
	case _EVENT_C:
		__poller_handle_connect(conn);
		break;
	case _EVENT_W:
		__poller_handle_send(conn);
		break;
	default:
		break;
	}
}

static void __poller_poll(int pfd) {

	event_t events[MAX_PROCESS_EVENTS];

	while (true) {

		int r = _event_wait(pfd, events, -1);
		for (int i = 0; i < r; i++) {

			poller_conn_t* conn = events[i].ptr;

			if (!conn->poller.tid) {
				conn->poller.tid = cdk_gettid();
			}
			if (!__rb_find_owner(&conn->owners, conn->poller.tid)) {

				inner_owner_t* owner = cdk_malloc(sizeof(inner_owner_t));
				owner->id = conn->poller.tid;

				__rb_insert_owner(&conn->owners, owner->id, &owner->n);
			}
			__poller_process_connection(conn);
		}
	}
}

static int __poller_slave(void* param) {

	int* ppfd = param;
	int  pfd  = *ppfd;

	__poller_poll(pfd);
	return 0;
}

void _poller_create(void) {

	thrd_t t;

	if (cdk_atomic_flag_test_and_set(&__once_create)) {
		return;
	}
	cdk_mtx_init(&__mutex);

	if (cdk_atomic_load(&__nslaves) == 0) {
		cdk_atomic_store(&__nslaves, cdk_cpus());
	}
	__master = epoll_create1(0);
	__slaves = cdk_malloc(sizeof(int) * cdk_atomic_load(&__nslaves));

	for (int i = 0; i < cdk_atomic_load(&__nslaves); i++) {

		*(__slaves + i) = epoll_create1(0);

		cdk_thrd_create(&t, __poller_slave, (__slaves + i));
		cdk_thrd_detach(t);
	}
	return;
}

void _poller_destroy(void) {

	cdk_mtx_destroy(&__mutex);

	_net_close(__master);

	for (int i = 0; i < cdk_atomic_load(&__nslaves); i++) {

		_net_close(*(__slaves + i));
	}
	if (__slaves) {
		cdk_free(__slaves);
	}
	return;
}

void _poller_concurrent_slaves(int64_t num) {

	if (num < 0) {
		num = 0;
	}
	cdk_atomic_store(&__nslaves, num);
}

void _poller_master(void) {

	if (cdk_atomic_flag_test_and_set(&__one_master)) {
		return;
	}
	__poller_poll(__master);
	return;
}

void _poller_setup_splicer(poller_conn_t* conn, splicer_profile_t* splicer) {

	memcpy(&conn->tcp.splicer, splicer, sizeof(splicer_profile_t));
}

poller_conn_t* _poller_conn_create(int pfd, sock_t s, int c, poller_handler_t* h) {

	poller_conn_t* conn = cdk_malloc(sizeof(poller_conn_t));

	conn->cmd   = c;
	conn->fd    = s;
	conn->h     = h;
	conn->type  = cdk_net_socktype(s);
	conn->poller.pfd   = pfd;
	conn->state = true;
	conn->poller.tid  = 0;

	cdk_rb_create(&(conn->owners));
	cdk_mtx_init(&conn->mutex);

	if (conn->type == SOCK_STREAM) {

		conn->tcp.ibuf.len = MAX_IOBUF_SIZE;
		conn->tcp.ibuf.off = 0;
		conn->tcp.ibuf.buf = cdk_malloc(MAX_IOBUF_SIZE);

		cdk_list_create(&(conn->tcp.olist));
	}
	if (conn->type == SOCK_DGRAM) {

		conn->udp.ibuf.len = MAX_IOBUF_SIZE;
		conn->udp.ibuf.off = 0;
		conn->udp.ibuf.buf = cdk_malloc(MAX_IOBUF_SIZE);

		cdk_list_create(&(conn->udp.olist));
	}
	switch (conn->cmd)
	{
	case _POLLER_CMD_A:
		_event_add(conn->poller.pfd, conn->fd, _EVENT_A, conn);
		break;
	case _POLLER_CMD_C:
		_event_add(conn->poller.pfd, conn->fd, _EVENT_C, conn);
		break;
	case _POLLER_CMD_R:
		_event_add(conn->poller.pfd, conn->fd, _EVENT_R, conn);
		break;
	case _POLLER_CMD_W:
		_event_add(conn->poller.pfd, conn->fd, _EVENT_W, conn);
		break;
	default:
		break;
	}
	return conn;
}

void _poller_conn_modify(poller_conn_t* conn) {

	switch (conn->cmd)
	{
	case _POLLER_CMD_A:
		_event_mod(conn->poller.pfd, conn->fd, _EVENT_A, conn);
		break;
	case _POLLER_CMD_C:
		_event_mod(conn->poller.pfd, conn->fd, _EVENT_C, conn);
		break;
	case _POLLER_CMD_R:
		_event_mod(conn->poller.pfd, conn->fd, _EVENT_R, conn);
		break;
	case _POLLER_CMD_W:
		_event_mod(conn->poller.pfd, conn->fd, _EVENT_W, conn);
		break;
	default:
		break;
	}
}

void _poller_conn_destroy(poller_conn_t* conn) {

	_event_del(conn->poller.pfd, conn->fd);
	_net_close(conn->fd);
	
	conn->state = false;

	inner_owner_t* entry =  __rb_find_owner(&conn->owners, cdk_gettid());
	cdk_rb_erase(&conn->owners, &entry->n);
	cdk_free(entry);

	if (conn->type == SOCK_STREAM) {
		cdk_free(conn->tcp.ibuf.buf);
		
		while (!cdk_list_empty(&(conn->tcp.olist))) {
			inner_offset_buf_t* e = cdk_list_data(cdk_list_head(&(conn->tcp.olist)), inner_offset_buf_t, n);
			cdk_list_remove(&(e->n));
			cdk_free(e);
		}
	}
	if (conn->type == SOCK_DGRAM) {
		cdk_free(conn->udp.ibuf.buf);

		while (!cdk_list_empty(&(conn->udp.olist))) {
			inner_offset_buf_t* e = cdk_list_data(cdk_list_head(&(conn->udp.olist)), inner_offset_buf_t, n);
			cdk_list_remove(&(e->n));
			cdk_free(e);
		}
	}
	if (!cdk_rb_first(&conn->owners)) {
		
		cdk_mtx_unlock(&conn->mutex);
		cdk_mtx_destroy(&conn->mutex);
		cdk_free(conn);
		return;
	}
	cdk_mtx_unlock(&conn->mutex);
	return;
}

poller_conn_t* _poller_listen(const char* restrict t, const char* restrict h, const char* restrict p, poller_handler_t* handler) {

	int            pfd;
	sock_t         s;
	poller_conn_t* conn;

	if (!strncmp(t, "tcp", strlen("tcp"))) {

		s    = _net_listen(h, p, SOCK_STREAM);
		conn = _poller_conn_create(__master, s, _POLLER_CMD_A, handler);
	}
	if (!strncmp(t, "udp", strlen("udp"))) {

		s    = _net_listen(h, p, SOCK_DGRAM);
		conn = _poller_conn_create(__poller_loadbalance(), s, _POLLER_CMD_R, handler);
	}
	return conn;
}

poller_conn_t* _poller_dial(const char* restrict t, const char* restrict h, const char* restrict p, poller_handler_t* handler) {

	int						pfd;
	sock_t					s;
	poller_conn_t*			conn;
	addrinfo_t				ai;
	struct sockaddr_storage ss;

	memset(&ai, 0, sizeof(addrinfo_t));
	memset(&ss, 0, sizeof(struct sockaddr_storage));

	if (!strncmp(t, "tcp", strlen("tcp"))) {

		s    = _net_dial(h, p, SOCK_STREAM);
		conn = _poller_conn_create(__poller_loadbalance(), s, _EVENT_C, handler);
	}
	if (!strncmp(t, "udp", strlen("udp"))) {

		s    = _net_dial(h, p, SOCK_DGRAM);
		conn = _poller_conn_create(__poller_loadbalance(), s, _EVENT_W, handler);

		memcpy(ai.a, h, strlen(h));
		ai.p = strtoul(p, NULL, 10);
		ai.f = cdk_net_af(s);

		cdk_net_inet_pton(&ai, &ss);

		conn->udp.peer.ss    = ss;
		conn->udp.peer.sslen = sizeof(struct sockaddr_storage);
	}
	return conn;
}

void _poller_postrecv(poller_conn_t* conn) {

	__poller_post_recv(conn);
	return;
}

void _poller_postsend(poller_conn_t* conn, void* data, size_t size) {

	cdk_mtx_lock(&conn->mutex);

	if (!__rb_find_owner(&conn->owners, cdk_gettid())) {

		inner_owner_t* owner = cdk_malloc(sizeof(inner_owner_t));
		owner->id = cdk_gettid();

		__rb_insert_owner(&conn->owners, owner->id, &owner->n);
	}
	if (!conn->state) {

		inner_owner_t* entry = __rb_find_owner(&conn->owners, cdk_gettid());
		cdk_rb_erase(&conn->owners, &entry->n);
		cdk_free(entry);

		if (!cdk_rb_first(&conn->owners)) {
			
			cdk_mtx_unlock(&conn->mutex);
			cdk_mtx_destroy(&conn->mutex);
			cdk_free(conn);
			return;
		}
		cdk_mtx_unlock(&conn->mutex);
		return;
	}
	inner_offset_buf_t* buffer = cdk_malloc(sizeof(inner_offset_buf_t) + size);

	memcpy(buffer->buf, data, size);
	buffer->len = size;
	buffer->off = 0;

	if (conn->type == SOCK_STREAM) {
		cdk_list_insert_tail(&(conn->tcp.olist), &(buffer->n));
	}
	if (conn->type == SOCK_DGRAM) {
		cdk_list_insert_tail(&(conn->udp.olist), &(buffer->n));
	}
	__poller_post_send(conn);

	cdk_mtx_unlock(&conn->mutex);
	return;
}

#endif