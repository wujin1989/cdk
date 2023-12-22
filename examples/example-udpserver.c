#include "cdk.h"

static void on_read(cdk_channel_t* channel, void* buf, size_t len) {
	char msg[4096] = { 0 };
	memcpy(msg, buf, len);
	printf("recv %s\n", msg);
}

static void on_close(cdk_channel_t* channel, const char* error) {
	printf("channel closed, reason: %s\n", error);
}

static void on_accept(cdk_channel_t* channel) {
	printf("udp accepted\n");
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
	cdk_handler_t handler = {
		.on_accept = on_accept,
		.on_read = on_read,
		.on_close = on_close,
		.tlsconf = &conf
	};
	cdk_net_listen(PROTOCOL_UDP, "0.0.0.0", "9999", &handler);
	
	cdk_net_cleanup();
	return 0;
}
