#include "cdk.h"

#define BUFFERSIZE 4096
#define RUNTIME	10   //s

cdk_timer_t timer;
int total_clients = 100;
int connected_clients;
int disconnected_clients;
int total_readcnt;
size_t total_readbytes;
char buffer[BUFFERSIZE];

static void handle_connect(cdk_channel_t* channel) {
	if (++connected_clients == total_clients) {
		cdk_logi("%d clients has connected.\n", total_clients);
	}
	cdk_net_send(channel, buffer, sizeof(buffer));
}

static void handle_read(cdk_channel_t* channel, void* buf, size_t len) {
	++total_readcnt;
	total_readbytes += len;
	cdk_net_send(channel, buf, len);
}

static void handle_close(cdk_channel_t* channel, const char* error) {
	if (++disconnected_clients == total_clients) {
		cdk_logi("%d clients has disconnected.\n", total_clients);
	}
}

static inline void _test_finished(void* param) {
	cdk_net_stop();
}

static void _printf_statistic_info() {
	cdk_logi("qps:\t %d/qps\n", total_readcnt / RUNTIME);
	cdk_logi("throughput:\t %zu MB/s\n", (total_readbytes / (RUNTIME * 1024 * 1024)));
}

int main(void) {
	cdk_net_startup(4);
	cdk_logger_create(NULL, 0);
	cdk_timer_create(&timer, 1);

	cdk_unpack_t unpacker = {
		.fixedlen.len = BUFFERSIZE,
	};
	cdk_handler_t handler = {
		.tcp.on_connect = handle_connect,
		.tcp.on_read = handle_read,
		.tcp.on_close = handle_close,
		.tcp.unpacker = &unpacker
	};
	for (int i = 0; i < total_clients; i++) {
		cdk_net_dial("tcp", "127.0.0.1", "9999", &handler);
	}
	cdk_timer_add(&timer, _test_finished, NULL, 10000, false);
	cdk_net_cleanup();
	cdk_timer_destroy(&timer);

	_printf_statistic_info();
	cdk_logger_destroy();
	return 0;
}
