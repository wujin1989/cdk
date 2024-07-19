#include "cdk.h"

typedef enum net_msg_type_e{
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

char ping[16] = "PING";
char keepalive[16] = "KEEPALIVE";

static void handle_connect(cdk_channel_t* channel) {
	cdk_logi("[%d]has connected to remote...\n", (int)channel->fd);
	net_msg_t* msg = malloc(sizeof(net_msg_t) + sizeof(ping));
	if (msg) {
		msg->hdr.size = htonl((uint32_t)(sizeof(ping)));
		msg->hdr.type = htonl(PING_MSG_TYPE);
		memcpy(msg->data, ping, sizeof(ping));
		cdk_net_send(channel, msg, sizeof(net_msg_t) + sizeof(ping));

		free(msg);
		msg = NULL;
	}
}

static void handle_read(cdk_channel_t* channel, void* buf, size_t len) {
	net_msg_t* rmsg = buf;
	cdk_address_t addrinfo;
	cdk_net_extract_address(channel->fd, &addrinfo, true);
	cdk_logi("recv %s from %s:%d. type: %d, len: %d.\n", rmsg->data, addrinfo.a, addrinfo.p, ntohl(rmsg->hdr.type), ntohl(rmsg->hdr.size));

	net_msg_t* msg = malloc(sizeof(net_msg_t) + sizeof(ping));
	if (msg) {
		msg->hdr.size = htonl((uint32_t)(sizeof(ping)));
		msg->hdr.type = htonl(PING_MSG_TYPE);
		memcpy(msg->data, ping, sizeof(ping));
		cdk_net_send(channel, msg, sizeof(net_msg_t) + sizeof(ping));

		free(msg);
		msg = NULL;
	}
}

static void handle_close(cdk_channel_t* channel, const char* error) {
	cdk_loge("connection closed, reason: %s\n", error);
}

static void handle_heartbeat(cdk_channel_t* channel) {
	net_msg_t* msg = malloc(sizeof(net_msg_t) + sizeof(keepalive));
	if (msg) {
		msg->hdr.size = htonl((uint32_t)(sizeof(keepalive)));
		msg->hdr.type = htonl(KEEPALIVE_MSG_TYPE);
		memcpy(msg->data, keepalive, sizeof(keepalive));
		cdk_net_send(channel, msg, sizeof(net_msg_t) + sizeof(keepalive));

		free(msg);
		msg = NULL;
	}
}

int main(void) {
	cdk_conf_t conf = {
		.nthrds = 4,
		.tls = {
			.cafile = "certs/ca.crt",
			.capath = NULL,
			.crtfile = NULL,
			.keyfile = NULL,
			.verifypeer = true
		}
	};
	cdk_net_startup(&conf);
	cdk_logger_create(NULL, false);

	cdk_unpacker_t unpacker = {
		.type = TYPE_LENGTHFIELD,
		.lengthfield.adj = 0,
		.lengthfield.coding = MODE_FIXEDINT,
		.lengthfield.offset = 0,
		.lengthfield.payload = 8,
		.lengthfield.size = 4
	};
	cdk_handler_t handler = {
		.tcp.on_connect = handle_connect,
		.tcp.on_read = handle_read,
		.tcp.on_close = handle_close,
		.tcp.on_heartbeat = handle_heartbeat,
		.tcp.wr_timeout = 10000,
		.tcp.conn_timeout = 5000,
		.tcp.hb_interval = 5000,
		.tcp.unpacker = &unpacker
	};
	cdk_net_dial("tcp", "127.0.0.1", "9999", &handler);
	cdk_net_cleanup();
	cdk_logger_destroy();
	return 0;
}
