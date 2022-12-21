#include "cdk.h"
#include <stdio.h>
#include <Windows.h>

#define	OP_READ 0
#define	OP_WRITE 1


HANDLE iocp;

typedef struct io_context_t {
	WSAOVERLAPPED               overlapped;
	char                        buf[8192];
	WSABUF                      wsabuf;
	int                         totalbytes;
	int                         sentbytes;
	int							op;
}io_context_t;

typedef struct sock_context_t {
	SOCKET                   socket;
	io_context_t             io_ctx;
} sock_context_t;

static int worker_thread(void* p) {

	int ret;
	DWORD size;
	sock_context_t* sock_ctx;
	io_context_t* io_ctx;
	LPWSAOVERLAPPED overlapped;
	printf("current thread [%d]\n", (int)cdk_gettid());
	while (true) {

		ret = GetQueuedCompletionStatus(iocp, &size, (PULONG_PTR)&sock_ctx, &overlapped, INFINITE);
		if (!ret) {
			printf("GetQueuedCompletionStatus() failed: %d\n", GetLastError());
		}
		if (!ret || (ret && (size == 0))) {

			cdk_net_close(sock_ctx->socket);
			continue;
		}
		io_ctx = (io_context_t*)overlapped;
		switch (io_ctx->op) {
		case OP_READ:

			printf("[%d]recv buffer: %s\n", (int)cdk_gettid(), io_ctx->wsabuf.buf);

			cdk_sleep(15000);
			DWORD flags = 0;
			memset(sock_ctx->io_ctx.wsabuf.buf, 0, sizeof(sock_ctx->io_ctx.wsabuf.buf));
			ret = WSARecv(sock_ctx->socket, &sock_ctx->io_ctx.wsabuf, 1, (LPDWORD)NULL, (LPDWORD)&flags, &sock_ctx->io_ctx.overlapped, NULL);
			if (ret == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError())) {
				printf("WSARecv() Failed: %d\n", WSAGetLastError());
				cdk_net_close(sock_ctx->socket);
			}
			break;

		case OP_WRITE:
			break;
		}
	}
	return 0;
}
static void _nonblock(sock_t s) {

	u_long on = 1;
	ioctlsocket(s, FIONBIO, &on);
}
static sock_t _tcp_accept(sock_t s) {

	sock_t c = accept(s, NULL, NULL);
	if (c == INVALID_SOCKET) {
		closesocket(s);
		return INVALID_SOCKET;
	}
	//_nonblock(c);
	return c;
}
int main(void) {
	int ret = 0;
	iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	sock_t s = cdk_tcp_listen("0.0.0.0", "9999");

	for (int i = 0; i < 4; i++) {
		thrd_t t;
		cdk_thrd_create(&t, worker_thread, NULL);
		cdk_thrd_detach(t);
	}
	sock_t c;
	while (true) {

		c = _tcp_accept(s);

		sock_context_t sock_ctx;
		memset(&sock_ctx, 0, sizeof(sock_ctx));
		sock_ctx.socket = c;
		sock_ctx.io_ctx.wsabuf.buf = sock_ctx.io_ctx.buf;
		sock_ctx.io_ctx.wsabuf.len = sizeof(sock_ctx.io_ctx.buf);
		sock_ctx.io_ctx.op = OP_READ;
		sock_ctx.io_ctx.sentbytes = 0;
		sock_ctx.io_ctx.totalbytes = 0;

		CreateIoCompletionPort((HANDLE)c, iocp, (ULONG_PTR)&sock_ctx, 0);
		DWORD flags = 0;
		ret = WSARecv(c, &sock_ctx.io_ctx.wsabuf, 1, (LPDWORD)NULL, (LPDWORD)&flags, &sock_ctx.io_ctx.overlapped, NULL);
		if (ret == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError())) {
			printf("WSARecv() Failed: %d\n", WSAGetLastError());
			cdk_net_close(sock_ctx.socket);
		}
	}
	cdk_net_close(c);
	cdk_net_close(s);
	CloseHandle(iocp);
	return 0;
}
