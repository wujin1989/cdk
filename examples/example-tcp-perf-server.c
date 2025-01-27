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

static void _close_cb(cdk_channel_t* channel, cdk_channel_error_t error) {
    if (++disconnected_clients == total_clients) {
        accepted_clients = disconnected_clients = 0;
        cdk_loge("%d clients has disconnected.\n", total_clients);
    }
}

int main(void) {
    cdk_unpacker_t unpacker = {
        .type = UNPACKER_TYPE_FIXEDLEN,
        .fixedlen.len = BUFFERSIZE,
    };
    cdk_handler_t handler = {
        .on_accept = _accept_cb,
        .on_read = _read_cb,
        .on_close = _close_cb,
        .unpacker = &unpacker};

    cdk_logger_config_t config = {
        .async = false,
    };
    cdk_logger_create(&config);
    cdk_net_listen("tcp", "0.0.0.0", "9999", &handler);
    getchar();
    cdk_logger_destroy();
    return 0;
}
