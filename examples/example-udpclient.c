#include "cdk.h"

static void on_read(cdk_channel_t* channel, void* buf, size_t len) {
}

static void on_close(cdk_channel_t* channel, const char* error) {
	printf("channel closed, reason: %s\n", error);
}

static int routine(void* p) {
	cdk_channel_t* channel = p;
	while (!atomic_load(&channel->closing)) {
		cdk_net_send(channel, "helloworld", strlen("helloworld") + 1);
		printf("sending\n");
	}
	return 0;
}

static void on_ready(cdk_channel_t* channel) {
	printf("udp ready\n");
	thrd_t tid;
	thrd_create(&tid, routine, channel);
	thrd_detach(tid);
}

int main(void) {
	cdk_net_startup(4);
	cdk_handler_t handler = {
		.udp.on_ready = on_ready,
		.udp.on_read = on_read,
		.udp.on_close = on_close,
	};
	cdk_net_dial("udp", "127.0.0.1", "9999", &handler);

	cdk_net_cleanup();
	return 0;
}
