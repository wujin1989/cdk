#include "cdk.h"

static void handle_connect(poller_conn_t* conn) {
	printf("[%d]has connected to remote...\n", (int)conn->fd);

	cdk_net_postsend(conn);
}

static void handle_write(poller_conn_t* conn) {

	printf("recv write envent\n");
	//cdk_net_postrecv(conn);
}
static void handle_read(poller_conn_t* conn) {

	for (list_node_t* n = cdk_list_head(&conn->rbufs); n != cdk_list_sentinel(&conn->rbufs); ) {

		conn_buf_t* buf = cdk_list_data(n, conn_buf_t, n);
		n = cdk_list_next(n);

		printf("[%d]recv %s\n", (int)conn->fd, buf->buf);

		cdk_list_remove(&buf->n);
		cdk_free(buf);
	}
	//cdk_net_postsend(conn);
	//cdk_net_postrecv(conn);
}

int main(void) {

	poller_handler_t handler = {
		.on_accept  = NULL,
		.on_connect = handle_connect,
		.on_read    = handle_read,
		.on_write   = handle_write
	};
	cdk_net_dial("tcp", "127.0.0.1", "9999", &handler);

	cdk_net_poller();
	return 0;
}