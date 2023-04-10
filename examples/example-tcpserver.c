#include "cdk.h"

typedef struct _net_msg_hdr_t {

	uint32_t      p_s;   /* payload size */
	uint32_t      p_t;   /* payload type */
}net_msg_hdr_t;

typedef struct _net_msg_t {

	net_msg_hdr_t      h;
	char               p[];
}net_msg_t;

static void handle_accept(cdk_net_conn_t* conn) {
	printf("tid[%d], [%d]new connection coming...\n", (int)cdk_utils_systemtid(), (int)conn->fd);

	cdk_unpack_t unpacker1 = {
		.type = UNPACK_TYPE_FIXEDLEN,
		.fixedlen.len = 6
	};
	cdk_unpack_t unpacker2 = {
		.type = UNPACK_TYPE_DELIMITER,
		.delimiter.delimiter = "\r\n\r\n"
	};
	cdk_unpack_t unpacker3 = {
		.type = UNPACK_TYPE_LENGTHFIELD,
		.lengthfield.adj = 0,
		.lengthfield.coding = LEN_FIELD_FIXEDINT,
		.lengthfield.offset = 0,
		.lengthfield.payload = 8,
		.lengthfield.size = 4
	};
	cdk_net_setup_unpacker(conn, &unpacker3);
	cdk_net_postrecv(conn);
}

static void handle_write(cdk_net_conn_t* conn, void* buf, size_t len) {

	net_msg_t* msg = (net_msg_t*)buf;
	printf("send complete. msg payload len: %d, msg payload type: %d, %s\n", ntohl(msg->h.p_s), ntohl(msg->h.p_t), msg->p);
	cdk_net_postrecv(conn);
}
static void handle_read(cdk_net_conn_t* conn, void* buf, size_t len) {

	net_msg_t* rmsg = (net_msg_t*)buf;
	printf("recv complete. msg payload len: %d, msg payload type: %d, %s\n", ntohl(rmsg->h.p_s), ntohl(rmsg->h.p_t), rmsg->p);


	net_msg_t* smsg = malloc(sizeof(net_msg_t) + strlen("world") + 1);
	smsg->h.p_s = htonl((unsigned long)(strlen("world") + 1));
	smsg->h.p_t = htonl(2);
	memcpy(smsg->p, "world", strlen("world") + 1);

	cdk_net_postsend(conn, smsg, sizeof(net_msg_t) + strlen("world") + 1);
}
static void handle_close(cdk_net_conn_t* conn) {

	printf("recv close\n");
	cdk_net_close(conn);
}
int main(void) {

	cdk_net_handler_t handler = {
		.on_accept  = handle_accept,
		.on_connect = NULL,
		.on_read    = handle_read,
		.on_write   = handle_write,
		.on_close   = handle_close
	};
	cdk_net_concurrent_slaves(4);
	cdk_net_listen("tcp", "0.0.0.0", "9999", &handler);
	
	cdk_net_poll();
	return 0;
}