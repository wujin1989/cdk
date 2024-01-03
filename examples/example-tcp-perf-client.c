#include "cdk.h"

#define BUFFERSIZE 4096
#define RUNTIME	10   //s

cdk_timer_t timer;
int total_clients = 100;
char buffer[BUFFERSIZE];
atomic_int connected_clients;
atomic_int disconnected_clients;
atomic_int total_readcnt;
atomic_size_t total_readbytes;

static void handle_connect(cdk_channel_t* channel) {
	atomic_fetch_add(&connected_clients, 1);
	if (atomic_load(&connected_clients) == total_clients) {
		cdk_logi("%d clients has connected.\n", total_clients);
	}
	cdk_net_send(channel, buffer, sizeof(buffer));
}

static void handle_read(cdk_channel_t* channel, void* buf, size_t len) {
	atomic_fetch_add(&total_readcnt, 1);
	atomic_fetch_add(&total_readbytes, len);
	cdk_net_send(channel, buf, len);
}

static void handle_close(cdk_channel_t* channel, const char* error) {
	atomic_fetch_add(&disconnected_clients, 1);
	if (atomic_load(&disconnected_clients) == total_clients) {
		cdk_logi("%d clients has disconnected.\n", total_clients);
	}
}

static inline void _finished(void* param) {
	cdk_net_exit();
}

static void _printf_statistic_info() {
	cdk_logi("tps:\t %d Q/s\n", atomic_load(&total_readcnt) / RUNTIME);
	cdk_logi("throughput:\t %zu MB/s\n", (atomic_load(&total_readbytes) / (RUNTIME * 1024 * 1024)));
}

int main(void) {
	cdk_net_startup(4);
	cdk_logger_create(NULL, false);
	cdk_timer_create();

	cdk_unpack_t unpacker = {
		.fixedlen.len = BUFFERSIZE,
	};
	cdk_handler_t handler = {
		.tcp.on_connect = handle_connect,
		.tcp.on_read = handle_read,
		.tcp.on_close = handle_close,
		.tcp.unpacker = &unpacker
	};
	atomic_init(&connected_clients, 0);
	atomic_init(&disconnected_clients, 0);
	atomic_init(&total_readcnt, 0);
	atomic_init(&total_readbytes, 0);

	for (int i = 0; i < total_clients; i++) {
		cdk_net_dial("tcp", "127.0.0.1", "9999", &handler);
	}
	cdk_timer_add(_finished, NULL, 10000, false);
	cdk_net_cleanup();
	cdk_timer_destroy();

	_printf_statistic_info();
	cdk_logger_destroy();
	return 0;
}
