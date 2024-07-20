#include "cdk.h"

static void _read_cb(cdk_channel_t* channel, void* buf, size_t len) {
	printf("[%d]\trecv %s\n", (int)cdk_utils_systemtid(), (char*)buf);
	cdk_net_send(channel, buf, len);
}

static void _close_cb(cdk_channel_t* channel, const char* error) {
	printf("channel closed, reason: %s\n", error);
}

int main(void) {
	cdk_conf_t conf = {
		.nthrds = 4
	};
	cdk_handler_t handler = {
		.udp.on_read = _read_cb,
		.udp.on_close = _close_cb,
	};
	cdk_net_startup(&conf);
	cdk_net_listen("udp", "0.0.0.0", "9999", &handler);
	cdk_net_cleanup();
	return 0;
}
