#include "cdk.h"

static void on_write(cdk_channel_t* channel, void* buf, size_t len) {
}

static void on_read(cdk_channel_t* channel, void* buf, size_t len) {
	cdk_addrinfo_t ai;
	cdk_net_ntop(&channel->udp.peer.ss, &ai);
	printf("%s from %s\n", (char*)buf, ai.a);
}

static void on_close(cdk_channel_t* channel, char* error) {
	printf("channel closed, reason: %s\n", error);
	cdk_net_close(channel);
}

static int routine(void* p) {
	cdk_channel_t* channel = p;
	while (true) {
		if (!cdk_net_send(channel, "helloworld", strlen("helloworld") + 1)) {
			break;
		}
	}
	return 0;
}

static int routine2(void* p) {
	cdk_channel_t* channel = p;
	cdk_net_close(channel);
	return 0;
}

static void on_ready(cdk_channel_t* channel) {
	thrd_t tid;
	thrd_create(&tid, routine, channel);
	thrd_detach(tid);

	thrd_t tid2;
	thrd_create(&tid2, routine2, channel);
	thrd_detach(tid2);
}

int main(void) {
	cdk_net_startup(4, NULL);
	
	cdk_handler_t handler = {
		.on_ready = on_ready,
		.on_read = on_read,
		.on_write = on_write,
		.on_close = on_close
	};
	cdk_net_dial("udp", "127.0.0.1", "9999", &handler);

	cdk_net_cleanup();
	return 0;
}
