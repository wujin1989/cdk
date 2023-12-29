#include "cdk.h"
#include <string.h>

typedef struct _net_msg_hdr_t {
	uint32_t p_s;
	uint32_t p_t;
}net_msg_hdr_t;

typedef struct _net_msg_t {
	net_msg_hdr_t h;
	char p[];
}net_msg_t;

static void handle_connect(cdk_channel_t* channel) {
	printf("[%d]has connected to remote...\n", (int)channel->fd);

	net_msg_t* smsg = malloc(sizeof(net_msg_t) + strlen("hello") + 1);
	if (!smsg) {
		return;
	}
	smsg->h.p_s = htonl((uint32_t)(strlen("hello") + 1));
	smsg->h.p_t = htonl(1);
	memcpy(smsg->p, "hello", strlen("hello") + 1);

	cdk_net_send(channel, smsg, sizeof(net_msg_t) + strlen("hello") + 1);
}

static void handle_read(cdk_channel_t* channel, void* buf, size_t len) {
	net_msg_t* rmsg = (net_msg_t*)buf;
	printf("recv complete. msg payload len: %d, msg payload type: %d, %s\n", ntohl(rmsg->h.p_s), ntohl(rmsg->h.p_t), rmsg->p);
}

static void handle_close(cdk_channel_t* channel, const char* error) {
	printf("connection closed, reason: %s\n", error);
}

int main(void) {
	cdk_net_startup(4);
	cdk_tlsconf_t conf = {
		.cafile = "certs/ca.crt",
		.capath = NULL,
		.crtfile = NULL,
		.keyfile = NULL,
		.verifypeer = true
	};
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
	cdk_handler_t handler = {
		.tcp.on_connect = handle_connect,
		.tcp.on_read    = handle_read,
		.tcp.on_close   = handle_close,
		.tcp.connect_timeout = 5000,
		.tcp.tlsconf = &conf,
		.tcp.unpacker = &unpacker3
	};
	cdk_net_dial("tcp", "127.0.0.1", "9999", &handler);
	
	cdk_net_cleanup();
	return 0;
}
