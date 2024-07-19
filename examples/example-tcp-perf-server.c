#include "cdk.h"

#define BUFFERSIZE 4096
int total_clients = 100;
int accepted_clients;
int disconnected_clients;

static void handle_accept(cdk_channel_t* channel) {
	if (++accepted_clients == total_clients) {
		cdk_logi("%d clients has accepted.\n", total_clients);
	}
}

static void handle_read(cdk_channel_t* channel, void* buf, size_t len) {
	cdk_net_send(channel, buf, len);
}

static void handle_close(cdk_channel_t* channel, const char* error) {
	if (++disconnected_clients == total_clients) {
		accepted_clients = disconnected_clients = 0;
		cdk_loge("%d clients has disconnected.\n", total_clients);
	}
}

int main(void) {
	cdk_conf_t conf = {
		.nthrds = 1
	};
	cdk_net_startup(&conf);
	cdk_logger_create(NULL, false);

	cdk_unpacker_t unpacker = {
		.fixedlen.len = BUFFERSIZE,
	};
	cdk_handler_t handler = {
		.tcp.on_accept = handle_accept,
		.tcp.on_read = handle_read,
		.tcp.on_close = handle_close,
		.tcp.unpacker = &unpacker
	};
	cdk_net_listen("tcp", "0.0.0.0", "9999", &handler);
	cdk_net_cleanup();
	cdk_logger_destroy();
	return 0;
}
