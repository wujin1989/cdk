#include "cdk.h"
#include <string.h>

typedef struct _net_msg_hdr_t {

	uint32_t      p_s;   /* payload size */
	uint32_t      p_t;   /* payload type */
}net_msg_hdr_t;

typedef struct _net_msg_t {

	net_msg_hdr_t      h;
	char               p[];
}net_msg_t;

static void handle_accept(poller_conn_t* conn) {
	printf("[%d]new connection coming...\n", (int)conn->fd);

	splicer_profile_t profile1 = {
		.type = SPLICE_TYPE_FIXED,
		.mfs  = 4096,
		.fixed.len = 6
	};
	splicer_profile_t profile2 = {
		.type = SPLICE_TYPE_TEXTPLAIN,
		.mfs = 4096,
		.textplain.delimiter = "\r\n\r\n"
	};
	splicer_profile_t profile3 = {
		.type = SPLICE_TYPE_BINARY,
		.mfs = 4096,
		.binary.adj = 0,
		.binary.coding = LEN_FIELD_FIXEDINT,
		.binary.offset = 0,
		.binary.payload = 8,
		.binary.size = 4
	};
	cdk_net_setup_splicer(conn, &profile3);
	cdk_net_postrecv(conn);
}

static void handle_write(poller_conn_t* conn, void* buf, size_t len) {

	net_msg_t* msg = (net_msg_t*)buf;
	printf("send complete. msg payload len: %d, msg payload type: %d, %s\n", ntohl(msg->h.p_s), ntohl(msg->h.p_t), msg->p);
	cdk_net_postrecv(conn);
}
static void handle_read(poller_conn_t* conn, void* buf, size_t len) {

	net_msg_t* rmsg = (net_msg_t*)buf;
	printf("recv complete. msg payload len: %d, msg payload type: %d, %s\n", ntohl(rmsg->h.p_s), ntohl(rmsg->h.p_t), rmsg->p);


	net_msg_t* smsg = cdk_malloc(sizeof(net_msg_t) + strlen("world") + 1);
	smsg->h.p_s = htonl(strlen("world") + 1);
	smsg->h.p_t = htonl(2);
	memcpy(smsg->p, "world", strlen("world") + 1);

	cdk_net_postsend(conn, smsg, sizeof(net_msg_t) + strlen("world") + 1);
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