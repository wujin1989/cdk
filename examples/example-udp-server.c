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

static void _close_cb(
    cdk_channel_t* channel, cdk_channel_reason_t code, const char* reason) {
    printf("channel closed, reason: %s\n", reason);
}

int main(void) {
    /*cdk_tls_conf_t conf = {
        .cafile = NULL,
        .capath = NULL,
        .crtfile = "certs/cert.crt",
        .keyfile = "certs/cert.key",
        .verifypeer = false,
        .dtls = true,
        .side = TLS_SIDE_SERVER};*/

    cdk_handler_t handler = {
        .udp.on_read  = _read_cb,
        .udp.on_close = _close_cb,
    };
    cdk_net_listen("udp", "0.0.0.0", "9999", &handler, 2, NULL);

    getchar();
    return 0;
}
