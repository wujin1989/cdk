#include "cdk.h"

typedef struct _net_msg_hdr_t {

	uint32_t      p_s;   /* payload size */
	uint32_t      p_t;   /* payload type */
}net_msg_hdr_t;

typedef struct _net_msg_t {

	net_msg_hdr_t      h;
	char               p[];
}net_msg_t;

static void handle_connect(poller_conn_t* conn) {
	printf("[%d]has connected to remote...\n", (int)conn->fd);

	cdk_net_postsend(conn);
}

static void handle_write(poller_conn_t* conn, void* buf, size_t len) {

	printf("recv write envent\n");
	//cdk_net_postrecv(conn);
}
static void handle_read(poller_conn_t* conn) {

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