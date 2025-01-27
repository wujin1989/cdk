#include "cdk.h"

#define BUFFERSIZE 4096
#define RUNTIME 10 // s

int           total_clients = 100;
char          buffer[BUFFERSIZE];
atomic_int    connected_clients;
atomic_int    disconnected_clients;
atomic_int    total_readcnt;
atomic_size_t total_readbytes;

static void _finished(void* param) {
    cdk_net_exit();
}

static void _statistic_info_printf() {
    cdk_logi("qps:\t %d Q/s\n", atomic_load(&total_readcnt) / RUNTIME);
    cdk_logi(
        "throughput:\t %zu MB/s\n",
        (atomic_load(&total_readbytes) / (RUNTIME * 1024 * 1024)));
}

static void _connect_cb(cdk_channel_t* channel) {
    atomic_fetch_add(&connected_clients, 1);
    if (atomic_load(&connected_clients) == total_clients) {
        cdk_logi("%d clients has connected.\n", total_clients);
    }
    cdk_net_send(channel, buffer, sizeof(buffer));
}

static void _read_cb(cdk_channel_t* channel, void* buf, size_t len) {
    atomic_fetch_add(&total_readcnt, 1);
    atomic_fetch_add(&total_readbytes, len);
    cdk_net_send(channel, buf, len);
}

static void _close_cb(cdk_channel_t* channel, cdk_channel_error_t error) {
    atomic_fetch_add(&disconnected_clients, 1);
    if (atomic_load(&disconnected_clients) == total_clients) {
        cdk_logi("%d clients has disconnected.\n", total_clients);
        _statistic_info_printf();
        cdk_logi("channel closed reason: %s\n", error.codestr);
    }
}

int main(void) {
    cdk_unpacker_t unpacker = {
        .type = UNPACKER_TYPE_FIXEDLEN,
        .fixedlen.len = BUFFERSIZE,
    };
    cdk_handler_t handler = {
        .on_connect = _connect_cb,
        .on_read = _read_cb,
        .on_close = _close_cb,
        .unpacker = &unpacker};

    cdk_logger_config_t config = {
        .async = false,
    };
    cdk_logger_create(&config);
    atomic_init(&connected_clients, 0);
    atomic_init(&disconnected_clients, 0);
    atomic_init(&total_readcnt, 0);
    atomic_init(&total_readbytes, 0);

    cdk_net_concurrency_configure(4);
    for (int i = 0; i < total_clients; i++) {
        cdk_net_dial("tcp", "127.0.0.1", "9999", &handler);
    }
    cdk_net_timer_create(_finished, NULL, 10000, false);

    getchar();
    cdk_logger_destroy();
    return 0;
}
