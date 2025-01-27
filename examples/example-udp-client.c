#include "cdk.h"

static void _read_cb(cdk_channel_t* channel, void* buf, size_t len) {
    cdk_address_t addrinfo;
    cdk_net_ntop(&channel->udp.peer.ss, &addrinfo);
    printf("recv %s from %s:%d\n", (char*)buf, addrinfo.addr, addrinfo.port);
}

static void _close_cb(cdk_channel_t* channel, cdk_channel_error_t error) {
    printf("channel closed, reason: %s\n", error.codestr);
}

static int _routine(void* p) {
    static int     num;
    cdk_channel_t* channel = p;
    while (true) {
        char buffer[64] = {0};
        sprintf(buffer, "%d", num++);
        if (!cdk_net_send(channel, buffer, sizeof(buffer))) {
            break;
        }
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
    cdk_handler_t handler = {
        .on_connect = _connect_cb,
        .on_read = _read_cb,
        .on_close = _close_cb,
    };
    cdk_net_dial("udp", "127.0.0.1", "9999", &handler);

    getchar();
    return 0;
}
