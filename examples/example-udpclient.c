#include "cdk.h"

static void handle_write(cdk_channel_t* channel, void* buf, size_t len) {
	cdk_net_channelrecv(channel);
}

static void handle_read(cdk_channel_t* channel, void* buf, size_t len) {
	cdk_addrinfo_t ai;
	cdk_net_ntop(&channel->udp.peer.ss, &ai);
	printf("%s from %s\n", (char*)buf, ai.a);
}

static void handle_close(cdk_channel_t* channel, char* error) {
	printf("channel closed, reason: %s\n", error);
	cdk_net_channelclose(channel);
}
static int routine(void* p) {
	cdk_channel_t* channel = p;
	while (true) {
		cdk_net_channelsend(channel, "helloworld", strlen("helloworld") + 1);
		cdk_time_sleep(1000);
	}
	return 0;
}
int main(void) {

	cdk_net_startup(1, 4, false);
	thrd_t tid;
	cdk_channel_t* channel;
	cdk_handler_t handler = {
		.on_read = handle_read,
		.on_write = handle_write,
		.on_close = handle_close
	};
	channel = cdk_net_dial("udp", "127.0.0.1", "9999", 0, &handler);
	
	thrd_create(&tid, routine, channel);
	thrd_detach(tid);

	cdk_net_poll();
	cdk_net_cleanup();
	return 0;
}