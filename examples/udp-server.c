#include "cdk.h"
#include <string.h>

static void handle_write(poller_conn_t* conn, void* buf, size_t len) {

}
static void handle_read(poller_conn_t* conn, void* buf, size_t len) {

	addrinfo_t ai;
	cdk_net_inet_ntop(&conn->udp.peer.ss, &ai);
	printf("recv %s from %s\n", (char*)buf, ai.a);
	//cdk_net_postrecv(conn);
}
static void handle_close(poller_conn_t* conn) {

	printf("server closed\n");
	cdk_net_close(conn);
}
int main(void) {

	poller_handler_t handler = {
		.on_accept = NULL,
		.on_connect = NULL,
		.on_read = handle_read,
		.on_write = NULL,
		.on_close = handle_close
	};
	cdk_net_concurrent_slaves(4);
	cdk_net_listen("udp", "0.0.0.0", "9999", &handler);
	cdk_net_poll();
	return 0;
}