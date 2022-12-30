#include "cdk.h"

static void handle_accept(poller_conn_t* conn) {
	printf("[%d]new connection coming...\n", (int)conn->fd);

	cdk_net_postrecv(conn);
}

static void handle_write(poller_conn_t* conn) {

	printf("recv write envent\n");
	cdk_net_postrecv(conn);
}
static void handle_read(poller_conn_t* conn) {
	
	printf("recv buf: %s\n", conn->iobuf.buffer.buf);
	
	cdk_net_postrecv(conn);
}

int main(void) {

	poller_handler_t handler = {
		.on_accept  = handle_accept,
		.on_connect = NULL,
		.on_read    = handle_read,
		.on_write   = handle_write
	};
	cdk_net_listen("tcp", "0.0.0.0", "9999", &handler);
	
	cdk_net_poller();
	return 0;
}