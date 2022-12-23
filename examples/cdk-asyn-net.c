#include "cdk.h"

static void handle_accept(sock_t sock) {
	printf("[%d]new connection coming...\n", (int)sock);
}

static void handle_read(poller_conn_t* conn) {
	
	conn_buf_t* buf = cdk_list_data(cdk_list_head(&conn->rbufs), conn_buf_t, n);
	cdk_list_remove(cdk_list_head(&conn->rbufs));
	
	printf("recv %s\n", buf->buf);
	cdk_free(buf);
	cdk_net_ctl(conn, _POLLER_CTL_R);
}

int main(void) {

	poller_handler_t handler = {
		.on_accept  = handle_accept,
		.on_connect = NULL,
		.on_read    = handle_read,
		.on_write   = NULL
	};
	cdk_net_listen("tcp", "0.0.0.0", "9999", &handler);
	
	cdk_net_poller();
	return 0;
}