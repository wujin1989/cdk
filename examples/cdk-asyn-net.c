#include "cdk.h"
#include <string.h>

static void handle_accept(poller_conn_t* conn) {
	printf("[%d]new connection coming...\n", (int)conn->fd);

	cdk_net_postrecv(conn);
}

static void handle_write(poller_conn_t* conn) {

	printf("recv write complete\n");
	cdk_net_postrecv(conn);
}
static void handle_read(poller_conn_t* conn) {
	
	char buf[1024];
	cdk_net_read(conn, buf, sizeof(buf));

	printf("recv buf: %s\n", buf);
	
	char* wbuf = "hello world\n";
	cdk_net_write(conn, wbuf, strlen(wbuf)+1);
	cdk_net_postsend(conn);
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