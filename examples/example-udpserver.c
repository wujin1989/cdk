#include "cdk.h"

static void on_read(cdk_channel_t* channel, void* buf, size_t len) {
	char msg[4096] = { 0 };
	memcpy(msg, buf, len);
	printf("recv %s\n", msg);
}

static void on_close(cdk_channel_t* channel, const char* error) {
	printf("channel closed, reason: %s\n", error);
}

static void on_ready(cdk_channel_t* channel) {
	printf("udp ready\n");
}

int main(void) {
	cdk_net_startup(4);
	cdk_handler_t handler = {
		.udp.on_ready = on_ready,
		.udp.on_read = on_read,
		.udp.on_close = on_close,
	};
	cdk_net_listen("udp", "0.0.0.0", "9999", &handler);
	
	cdk_net_cleanup();
	return 0;
}
