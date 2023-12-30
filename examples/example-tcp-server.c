#include "cdk.h"

typedef struct _net_msg_hdr_t {
	uint32_t      p_s;   /* payload size */
	uint32_t      p_t;   /* payload type */
}net_msg_hdr_t;

typedef struct _net_msg_t {
	net_msg_hdr_t      h;
	char               p[];
}net_msg_t;

static void handle_accept(cdk_channel_t* channel) {
	printf("tid[%d], [%d]new connection coming...\n", (int)cdk_utils_systemtid(), (int)channel->fd);
}

static void handle_read(cdk_channel_t* channel, void* buf, size_t len) {
	net_msg_t* rmsg = (net_msg_t*)buf;
	printf("recv complete. msg payload len: %d, msg payload type: %d, %s\n", ntohl(rmsg->h.p_s), ntohl(rmsg->h.p_t), rmsg->p);

	net_msg_t* smsg = malloc(sizeof(net_msg_t) + strlen("world") + 1);
	if (smsg) {
		smsg->h.p_s = htonl((unsigned long)(strlen("world") + 1));
		smsg->h.p_t = htonl(2);
		memcpy(smsg->p, "world", strlen("world") + 1);

		cdk_net_send(channel, smsg, sizeof(net_msg_t) + strlen("world") + 1);
	}
}

static void handle_close(cdk_channel_t* channel, const char* error) {
	printf("connection closed, reason: %s\n", error);
}

int main(void) {
    cdk_net_startup(4);
	
	cdk_tlsconf_t conf = {
		.cafile = NULL,
		.capath = NULL,
		.crtfile = "certs/server.crt",
		.keyfile = "certs/server.key",
		.verifypeer = false
	};
	cdk_unpack_t unpacker = {
		.type = UNPACK_TYPE_LENGTHFIELD,
		.lengthfield.adj = 0,
		.lengthfield.coding = LEN_FIELD_FIXEDINT,
		.lengthfield.offset = 0,
		.lengthfield.payload = 8,
		.lengthfield.size = 4
	};
	cdk_handler_t handler = {
		.tcp.on_accept  = handle_accept,
		.tcp.on_read    = handle_read,
		.tcp.on_close   = handle_close,
		.tcp.connect_timeout = 0,
		.tcp.tlsconf = &conf,
		.tcp.unpacker = &unpacker
	};
	cdk_net_listen("tcp", "0.0.0.0", "9999", &handler);
	
	cdk_net_cleanup();
	return 0;
}
