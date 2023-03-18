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

#include "win-poller.h"
#include "win-net.h"
#include "cdk/cdk-net.h"
#include "cdk/cdk-memory.h"
#include <stdlib.h>

#define _POLLER_CMD_R    0x1
#define _POLLER_CMD_W    0x2
#define _POLLER_CMD_A    0x4
#define _POLLER_CMD_C    0x8

static HANDLE iocp;

void _poller_create(void) {

	if (iocp == NULL) {
		iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	}
	return;
}

void _poller_destroy(void) {

	if (iocp) {
		CloseHandle(iocp);
	}
	return;
}

poller_conn_t* _poller_conn_create(sock_t s, uint32_t c, poller_handler_t* h) {

	poller_conn_t* conn = cdk_malloc(sizeof(poller_conn_t));

	WSAOVERLAPPED overlapped = { 0 };

	conn->cmd = c;
	conn->fd  = s;
	conn->h   = h;
	conn->o   = overlapped;

	memset(&conn->iobuf, 0, sizeof(conn_buf_t));

	CreateIoCompletionPort((HANDLE)s, iocp, (ULONG_PTR)conn, 0);

	return conn;
}

void _poller_conn_modify(poller_conn_t* conn) {

}

void _poller_conn_destroy(poller_conn_t* conn) {

	_net_close(conn->fd);
	cdk_free(conn->iobuf.buffer.buf);
	cdk_free(conn);
}

void _poller_poll(void) {
	
	LPWSAOVERLAPPED overlapped;
	poller_conn_t   conn;
	DWORD           size;
	while (true) {

		int r = GetQueuedCompletionStatus(iocp, &size, (PULONG_PTR)&conn, &overlapped, INFINITE);
		if (!r) {
			abort();
		}
		if (conn.cmd & _POLLER_CMD_A) {

		}
		if (conn.cmd & _POLLER_CMD_R) {

		}
		if (conn.cmd & _POLLER_CMD_C) {

		}
		if (conn.cmd & _POLLER_CMD_W) {

		}
	}
}

void _poller_listen(const char* restrict t, const char* restrict h, const char* restrict p, poller_handler_t* handler) {

	sock_t s;
	_poller_create();

	s = INVALID_SOCKET;
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

	s = INVALID_SOCKET;
	connected = false;
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

	conn->cmd   = _POLLER_CMD_R;
	
	DWORD flags = 0;
	int r = WSARecv(conn->fd, &conn->iobuf.buffer, 1, (LPDWORD)NULL, (LPDWORD)&flags, &conn->o, NULL);
	if (r == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError())) {
		_net_close(conn->fd);
	}
}

void _poller_post_send(poller_conn_t* conn) {

	conn->cmd = _POLLER_CMD_W;

	DWORD flags = 0;
	int r = WSASend(conn->fd, &conn->iobuf.buffer, 1, (LPDWORD)NULL, (LPDWORD)&flags, &conn->o, NULL);
	if (r == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError())) {
		_net_close(conn->fd);
	}
}