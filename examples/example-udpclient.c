#include "cdk.h"

static void handle_write(cdk_net_conn_t* conn, void* buf, size_t len) {
	cdk_net_postrecv(conn);
}

static void handle_read(cdk_net_conn_t* conn, void* buf, size_t len) {
	cdk_addrinfo_t ai;
	cdk_net_ntop(&conn->udp.peer.ss, &ai);
	printf("%s from %s\n", (char*)buf, ai.a);
}

static void handle_close(cdk_net_conn_t* conn, char* error) {
	printf("connection closed, reason: %s\n", error);
	cdk_net_close(conn);
}
static int routine(void* p) {
	cdk_net_conn_t* conn = p;
	while (true) {
		cdk_net_postsend(conn, "helloworld", strlen("helloworld") + 1);
		cdk_time_sleep(1000);
	}
	return 0;
}
int main(void) {

	thrd_t tid;
	cdk_net_conn_t* conn;

	cdk_net_handler_t handler = {
		.on_read = handle_read,
		.on_write = handle_write,
		.on_close = handle_close
	};
	cdk_net_concurrent_slaves(4);
	conn = cdk_net_dial("udp", "127.0.0.1", "9999", 0, &handler);
	
	thrd_create(&tid, routine, conn);
	thrd_detach(tid);

	cdk_net_poll();
	return 0;
}