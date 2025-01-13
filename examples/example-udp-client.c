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
    static int     num;
    cdk_channel_t* channel = p;
    while (!atomic_load(&channel->closing)) {
        char buffer[64] = {0};
        sprintf(buffer, "%d", num++);
        cdk_net_send(channel, buffer, sizeof(buffer));
        cdk_time_sleep(1000);
    }
    return 0;
}

static void _connect_cb(cdk_channel_t* channel) {
    printf("udp connected\n");
    thrd_t tid;
    thrd_create(&tid, _routine, channel);
    thrd_detach(tid);
}

int main(void) {
    cdk_net_conf_t conf = {
        .nthrds = 1,
        .dtls = {
            .cafile = "certs/ca.crt",
            .capath = NULL,
            .crtfile = NULL,
            .keyfile = NULL,
            .verifypeer = true,
            .dtls = true,
            .side = SIDE_CLIENT}};
    cdk_handler_t handler = {
        .udp.on_connect = _connect_cb,
        .udp.on_read = _read_cb,
        .udp.on_close = _close_cb,
    };
    cdk_net_startup(&conf);
    cdk_net_dial("udp", "127.0.0.1", "9999", &handler);
    cdk_net_cleanup();
    return 0;
}
