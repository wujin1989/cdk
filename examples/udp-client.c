#include "cdk.h"
#include <string.h>

static void handle_write(poller_conn_t* conn, void* buf, size_t len) {

	printf("send complete.//// %s\n", (char*)buf);
}

static void handle_read(poller_conn_t* conn, void* buf, size_t len) {
	
}
static void handle_close(poller_conn_t* conn) {

	printf("client close\n");
	cdk_net_close(conn);
}
static int video_thrd(void* p) {

	poller_conn_t* conn = p;
	char buf[2048] = { 0 };
	static size_t num = 0;
	while (true) {
		sprintf(buf, "hello_%zu", num++);
		
		cdk_net_postsend(conn, buf, sizeof(buf));
		//cdk_sleep(1000);
		if (!conn->state) {
			break;
		}
	}
	printf("video_thrd exit\n");
	return 0;
}
int main(void) {

	poller_conn_t* conn;
	thrd_t t;

	poller_handler_t handler = {
		.on_accept = NULL,
		.on_connect = NULL,
		.on_read = NULL,
		.on_write = handle_write,
		.on_close = handle_close
	};
	cdk_net_concurrent_slaves(4);
	conn = cdk_net_dial("udp", "127.0.0.1", "9999", &handler);
	
	cdk_thrd_create(&t, video_thrd, conn);
	cdk_thrd_detach(t);

	cdk_net_poll();
	return 0;
}