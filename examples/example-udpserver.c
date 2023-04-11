#include "cdk.h"

atomic_flag once = ATOMIC_FLAG_INIT;

int task(void* p) {
	cdk_net_conn_t* conn = p;
	char buf[2048] = { 0 };
	static size_t num = 0;
	while (true) {
		sprintf(buf, "world_%zu", num++);
		cdk_net_postsend(conn, buf, sizeof(buf));
		cdk_time_sleep(500);
	}
	cdk_net_close(conn);
	return 0;
}

static void handle_write(cdk_net_conn_t* conn, void* buf, size_t len) {
	printf("send complete.//// %s\n", (char*)buf);
	cdk_net_postrecv(conn);
}
static void handle_read(cdk_net_conn_t* conn, void* buf, size_t len) {

	cdk_addrinfo_t ai;
	cdk_net_ntop(&conn->udp.peer.ss, &ai);
	printf("recv %s from %s\n", (char*)buf, ai.a);
	
	if (!atomic_flag_test_and_set(&once)) {
		thrd_t tid;
		thrd_create(&tid, task, conn);
		thrd_detach(tid);
	}
}

static void handle_error(cdk_net_conn_t* conn, int err) {

	printf("error occurs , errno: %d ...\n", err);
	cdk_net_close(conn);
}
int main(void) {

	cdk_net_handler_t handler = {
		.on_read = handle_read,
		.on_write = handle_write,
		.on_error = handle_error
	};
	cdk_net_concurrent_slaves(4);
	cdk_net_listen("udp", "0.0.0.0", "9999", &handler);
	cdk_net_poll();
	return 0;
}