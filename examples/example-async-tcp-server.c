#include "cdk.h"

typedef enum msg_type_e {
    TYPE_KEEPALIVE,
    TYPE_PING,
    TYPE_PONG,
} msg_type_t;

typedef struct msg_hdr_s {
    uint32_t   size;
    msg_type_t type;
} msg_hdr_t;

typedef struct msg_s {
    msg_hdr_t hdr;
    char      data[];
} msg_t;

char PONG[16] = "PONG";
char KEEPALIVE[16] = "KEEPALIVE";

static void _accept_cb(cdk_channel_t* channel) {
    cdk_logi(
        "tid[%d], [%d]new connection coming...\n", (int)cdk_utils_systemtid(),
        (int)channel->fd);
}

static void _read_cb(cdk_channel_t* channel, void* buf, size_t len) {
    msg_t*        rmsg = buf;
    cdk_address_t addrinfo;
    cdk_net_address_retrieve(channel->fd, &addrinfo, true);
    cdk_logi(
        "recv %s from %s:%d. type: %d, len: %d.\n", rmsg->data, addrinfo.addr,
        addrinfo.port, ntohl(rmsg->hdr.type), ntohl(rmsg->hdr.size));

    if (ntohl(rmsg->hdr.type) == TYPE_PING) {
        msg_t* smsg = malloc(sizeof(msg_t) + sizeof(PONG));
        if (smsg) {
            smsg->hdr.size = htonl((uint32_t)(sizeof(PONG)));
            smsg->hdr.type = htonl(TYPE_PONG);
            memcpy(smsg->data, PONG, sizeof(PONG));
            cdk_net_send(channel, smsg, sizeof(msg_t) + sizeof(PONG));
            free(smsg);
            smsg = NULL;
        }
    }
}

static void _close_cb(
    cdk_channel_t* channel, cdk_channel_reason_t code, const char* reason) {
    cdk_loge("connection closed, reason: %s\n", reason);
}

static void _heartbeat_cb(cdk_channel_t* channel) {
    msg_t* smsg = malloc(sizeof(msg_t) + sizeof(KEEPALIVE));
    if (smsg) {
        smsg->hdr.size = htonl((uint32_t)(sizeof(KEEPALIVE)));
        smsg->hdr.type = htonl(TYPE_KEEPALIVE);
        memcpy(smsg->data, KEEPALIVE, sizeof(KEEPALIVE));
        cdk_net_send(channel, smsg, sizeof(msg_t) + sizeof(KEEPALIVE));
        free(smsg);
        smsg = NULL;
    }
}

int main(void) {
    cdk_net_conf_t conf = {
        .nthrds = 4,
        .tls = {
            .cafile = NULL,
            .capath = NULL,
            .crtfile = "certs/cert.crt",
            .keyfile = "certs/cert.key",
            .verifypeer = false,
            .dtls = false,
            .side = SIDE_SERVER}};

    cdk_unpacker_t unpacker = {
        .type = TYPE_LENGTHFIELD,
        .lengthfield.adj = 0,
        .lengthfield.coding = MODE_FIXEDINT,
        .lengthfield.offset = 0,
        .lengthfield.payload = 8,
        .lengthfield.size = 4};

    cdk_handler_t handler = {
        .tcp.on_accept = _accept_cb,
        .tcp.on_read = _read_cb,
        .tcp.on_close = _close_cb,
        /*.tcp.on_heartbeat = _heartbeat_cb,
        .tcp.rd_timeout = 10000,
        .tcp.hb_interval = 5000,*/
        .tcp.unpacker = &unpacker};

    cdk_net_startup(&conf);
    cdk_logger_config_t config = {
        .async = false,
    };
    cdk_logger_create(&config);
    cdk_net_listen("tcp", "0.0.0.0", "9999", &handler);
    cdk_net_cleanup();
    cdk_logger_destroy();
    return 0;
}
