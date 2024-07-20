#include "cdk.h"

static void _read_cb(cdk_channel_t* channel, void* buf, size_t len) {
	cdk_address_t addrinfo;
	cdk_net_ntop(&channel->udp.peer.ss, &addrinfo);
	printf("recv %s from %s:%d\n", (char*)buf, addrinfo.addr, addrinfo.port);
}

static void _close_cb(cdk_channel_t* channel, const char* error) {
	printf("channel closed, reason: %s\n", error);
}

static int _routine(void* p) {
	static int num;
	cdk_channel_t* channel = p;
	while (!atomic_load(&channel->closing)) {
		char buffer[64] = { 0 };
		sprintf(buffer, "%d", num++);
		cdk_net_send(channel, buffer, sizeof(buffer));
	}
	return 0;
}

static void _ready_cb(cdk_channel_t* channel) {
	printf("udp ready\n");
	thrd_t tid;
	thrd_create(&tid, _routine, channel);
	thrd_detach(tid);
}

int main(void) {
	cdk_conf_t conf = {
		.nthrds = 4
	};
	cdk_handler_t handler = {
		.udp.on_ready = _ready_cb,
		.udp.on_read = _read_cb,
		.udp.on_close = _close_cb,
	};
	cdk_net_startup(&conf);
	cdk_net_dial("udp", "127.0.0.1", "9999", &handler);
	cdk_net_cleanup();
	return 0;
}
