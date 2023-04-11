#include "cdk.h"

static void handle_write(cdk_net_conn_t* conn, void* buf, size_t len) {

	printf("send complete.//// %s\n", (char*)buf);
	cdk_net_postrecv(conn);
}

static void handle_read(cdk_net_conn_t* conn, void* buf, size_t len) {
	cdk_addrinfo_t ai;
	cdk_net_ntop(&conn->udp.peer.ss, &ai);
	printf("recv %s from %s\n", (char*)buf, ai.a);
	
}

static void handle_error(cdk_net_conn_t* conn, int err) {

	printf("error occurs , errno: %d ...\n", err);
	cdk_net_close(conn);
}
static int video_thrd(void* p) {

	cdk_net_conn_t* conn = p;
	char buf[2048] = { 0 };
	static size_t num = 0;
	while (true) {
		sprintf(buf, "hello_%zu", num++);
		cdk_net_postsend(conn, buf, sizeof(buf));
		cdk_time_sleep(500);
	}
	cdk_net_close(conn);
	printf("video_thrd exit\n");
	return 0;
}
int main(void) {

	cdk_net_conn_t* conn;
	thrd_t t;

	cdk_net_handler_t handler = {
		.on_read = handle_read,
		.on_write = handle_write,
		.on_error = handle_error
	};
	cdk_net_concurrent_slaves(4);
	conn = cdk_net_dial("udp", "127.0.0.1", "9999", 0, &handler);
	
	thrd_create(&t, video_thrd, conn);
	thrd_detach(t);

	cdk_net_poll();
	return 0;
}