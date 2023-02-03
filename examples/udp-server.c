#include "cdk.h"
#include <string.h>

static void handle_accept(poller_conn_t* conn) {
	printf("tid[%d], [%d]new connection coming...\n", (int)cdk_gettid(), (int)conn->fd);
	//cdk_net_postrecv(conn);
}
static void handle_write(poller_conn_t* conn, void* buf, size_t len) {

}
static void handle_read(poller_conn_t* conn, void* buf, size_t len) {

	printf("recv %s\n", (char*)buf);
	//cdk_net_postrecv(conn);
}
static void handle_close(poller_conn_t* conn) {

	cdk_net_close(conn);
}
int main(void) {

	poller_handler_t handler = {
		.on_accept = handle_accept,
		.on_connect = NULL,
		.on_read = handle_read,
		.on_write = handle_write,
		.on_close = handle_close
	};
	cdk_net_listen("udp", "0.0.0.0", "9999", &handler);

	cdk_net_poller();
	return 0;
}