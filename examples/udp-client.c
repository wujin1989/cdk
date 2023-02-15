#include "cdk.h"
#include <string.h>

static void handle_write(poller_conn_t* conn, void* buf, size_t len) {
	printf("%s  ----> send ok.\n", (char*)buf);

	cdk_net_postsend(conn, "hello", strlen("hello") + 1);
}

static void handle_read(poller_conn_t* conn, void* buf, size_t len) {
	
}
static void handle_close(poller_conn_t* conn) {


	cdk_net_close(conn);
}
int main(void) {

	poller_handler_t handler = {
		.on_accept = NULL,
		.on_connect = NULL,
		.on_read = NULL,
		.on_write = handle_write,
		.on_close = handle_close
	};
	cdk_net_concurrent_slaves(4);
	cdk_net_dial("udp", "127.0.0.1", "9999", &handler);
	cdk_net_poll();
	return 0;
}