#include "cdk.h"

typedef enum net_msg_type_e {
	KEEPALIVE_MSG_TYPE,
	PING_MSG_TYPE,
	PONG_MSG_TYPE,
}net_msg_type_t;

typedef struct net_msg_hdr_s {
	uint32_t size;
	net_msg_type_t type;
}net_msg_hdr_t;

typedef struct net_msg_s {
	net_msg_hdr_t hdr;
	char data[];
}net_msg_t;

char pong[16] = "PONG";
char keepalive[16] = "KEEPALIVE";

static void handle_accept(cdk_channel_t* channel) {
	cdk_logi("tid[%d], [%d]new connection coming...\n", (int)cdk_utils_systemtid(), (int)channel->fd);
}

static void handle_read(cdk_channel_t* channel, void* buf, size_t len) {
	net_msg_t* rmsg = buf;
	cdk_addrinfo_t addrinfo;
	cdk_net_obtain_addr(channel->fd, &addrinfo, true);
	cdk_logi("recv %s from %s:%d. type: %d, len: %d.\n", rmsg->data, addrinfo.a, addrinfo.p, ntohl(rmsg->hdr.type), ntohl(rmsg->hdr.size));

	if (ntohl(rmsg->hdr.type) == PING_MSG_TYPE) {
		net_msg_t* smsg = malloc(sizeof(net_msg_t) + sizeof(pong));
		if (smsg) {
			smsg->hdr.size = htonl((uint32_t)(sizeof(pong)));
			smsg->hdr.type = htonl(PONG_MSG_TYPE);
			memcpy(smsg->data, pong, sizeof(pong));
			cdk_net_send(channel, smsg, sizeof(net_msg_t) + sizeof(pong));
		}
	}
}

static void handle_close(cdk_channel_t* channel, const char* error) {
	cdk_loge("connection closed, reason: %s\n", error);
}

static void handle_timeout(cdk_channel_t* channel) {
	cdk_loge("connection timeout.\n");
	cdk_net_close(channel);
}

int main(void) {
    cdk_net_startup(4);
	cdk_logger_create(NULL, 0);
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
		.tcp.on_timeout = handle_timeout,
		.tcp.rd_timeout = 10000,
		.tcp.tlsconf = &conf,
		.tcp.unpacker = &unpacker
	};
	cdk_net_listen("tcp", "0.0.0.0", "9999", &handler);
	cdk_net_cleanup();
	cdk_logger_destroy();
	return 0;
}