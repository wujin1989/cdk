#include "cdk.h"

static void on_read(cdk_channel_t* channel, void* buf, size_t len) {
	cdk_address_t addrinfo;
	cdk_net_ntop(&channel->udp.peer.ss, &addrinfo);
	printf("recv %s from %s:%d\n", (char*)buf, addrinfo.a, addrinfo.p);
}

static void on_close(cdk_channel_t* channel, const char* error) {
	printf("channel closed, reason: %s\n", error);
}

static int routine(void* p) {
	static int num;
	cdk_channel_t* channel = p;

	while (!atomic_load(&channel->closing)) {
		char buffer[64] = { 0 };
		sprintf(buffer, "%d", num++);
		cdk_net_send(channel, buffer, sizeof(buffer));
	}
	return 0;
}

static void on_connect(cdk_channel_t* channel) {
	printf("udp connected\n");
	thrd_t tid;
	thrd_create(&tid, routine, channel);
	thrd_detach(tid);
}

int main(void) {
	cdk_conf_t conf = {
		.nthrds = 4
	};
	cdk_net_startup(&conf);
	cdk_handler_t handler = {
		.udp.on_connect = on_connect,
		.udp.on_read = on_read,
		.udp.on_close = on_close,
	};
	cdk_net_dial("udp", "127.0.0.1", "9999", &handler);
	cdk_net_cleanup();
	return 0;
}
