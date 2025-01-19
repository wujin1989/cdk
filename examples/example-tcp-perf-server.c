#include "cdk.h"

#define BUFFERSIZE 4096
int total_clients = 100;
int accepted_clients;
int disconnected_clients;

static void _accept_cb(cdk_channel_t* channel) {
    if (++accepted_clients == total_clients) {
        cdk_logi("%d clients has accepted.\n", total_clients);
    }
}

static void _read_cb(cdk_channel_t* channel, void* buf, size_t len) {
    cdk_net_send(channel, buf, len);
}

static void _close_cb(cdk_channel_t* channel, cdk_channel_reason_t code, const char* reason) {
    if (++disconnected_clients == total_clients) {
        accepted_clients = disconnected_clients = 0;
        cdk_loge("%d clients has disconnected.\n", total_clients);
    }
}

int main(void) {
    cdk_unpacker_t unpacker = {
        .fixedlen.len = BUFFERSIZE,
    };
    cdk_handler_t handler = {
        .tcp.on_accept = _accept_cb,
        .tcp.on_read = _read_cb,
        .tcp.on_close = _close_cb,
        .tcp.unpacker = &unpacker};

    cdk_logger_config_t config = {
        .async = false,
    };
    cdk_logger_create(&config);
    cdk_net_listen("tcp", "0.0.0.0", "9999", &handler, 1, NULL);
    getchar();
    cdk_logger_destroy();
    return 0;
}
