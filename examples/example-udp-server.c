#include "cdk.h"

static void _read_cb(cdk_channel_t* channel, void* buf, size_t len) {
    cdk_address_t addrinfo;
    cdk_net_ntop(&channel->udp.peer.ss, &addrinfo);
    printf(
        "[%d]\trecv %s from %s:%d\n",
        (int)cdk_utils_systemtid(),
        (char*)buf,
        addrinfo.addr,
        addrinfo.port);
    cdk_net_send(channel, buf, len);
}

static void _close_cb(cdk_channel_t* channel, cdk_channel_error_t error) {
    printf("channel closed, reason: %s\n", error.codestr);
}

int main(void) {
    cdk_handler_t handler = {
        .on_read  = _read_cb,
        .on_close = _close_cb,
    };
    cdk_net_concurrency_configure(4);
    cdk_net_listen("udp", "0.0.0.0", "9999", &handler);

    getchar();
    cdk_net_exit();
    return 0;
}
