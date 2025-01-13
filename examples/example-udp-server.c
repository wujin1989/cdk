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

static void _close_cb(cdk_channel_t* channel, const char* error) {
    printf("channel closed, reason: %s\n", error);
}

int main(void) {
    cdk_net_conf_t conf = {
        .nthrds = 4,
        .dtls   = {
              .cafile     = NULL,
              .capath     = NULL,
              .crtfile    = "certs/cert.crt",
              .keyfile    = "certs/cert.key",
              .verifypeer = false,
              .dtls       = true,
              .side       = SIDE_SERVER}};

    cdk_handler_t handler = {
        .udp.on_read  = _read_cb,
        .udp.on_close = _close_cb,
    };
    cdk_net_startup(&conf);
    cdk_net_listen("udp", "0.0.0.0", "9999", &handler);
    cdk_net_cleanup();
    return 0;
}
