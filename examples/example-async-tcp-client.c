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

char PING[16] = "PING";
char KEEPALIVE[16] = "KEEPALIVE";

static void _connect_cb(cdk_channel_t* channel) {
    cdk_logi("[%d]has connected to remote...\n", (int)channel->fd);
    msg_t* smsg = malloc(sizeof(msg_t) + sizeof(PING));
    if (smsg) {
        smsg->hdr.size = htonl((uint32_t)(sizeof(PING)));
        smsg->hdr.type = htonl(TYPE_PING);
        memcpy(smsg->data, PING, sizeof(PING));
        cdk_net_send(channel, smsg, sizeof(msg_t) + sizeof(PING));
        free(smsg);
        smsg = NULL;
    }
}

static void _read_cb(cdk_channel_t* channel, void* buf, size_t len) {
    msg_t*        rmsg = buf;
    cdk_address_t addrinfo;
    cdk_net_address_retrieve(channel->fd, &addrinfo, true);
    cdk_logi(
        "recv %s from %s:%d. type: %d, len: %d.\n", rmsg->data, addrinfo.addr,
        addrinfo.port, ntohl(rmsg->hdr.type), ntohl(rmsg->hdr.size));

    msg_t* smsg = malloc(sizeof(msg_t) + sizeof(PING));
    if (smsg) {
        smsg->hdr.size = htonl((uint32_t)(sizeof(PING)));
        smsg->hdr.type = htonl(TYPE_PING);
        memcpy(smsg->data, PING, sizeof(PING));
        cdk_net_send(channel, smsg, sizeof(msg_t) + sizeof(PING));
        free(smsg);
        smsg = NULL;
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
    cdk_tls_conf_t conf = {
        .cafile = "certs/ca.crt",
        .capath = NULL,
        .crtfile = NULL,
        .keyfile = NULL,
        .verifypeer = true,
        .dtls = false,
        .side = TLS_SIDE_CLIENT};

    cdk_unpacker_t unpacker = {
        .type = UNPACKER_TYPE_LENGTHFIELD,
        .lengthfield.adj = 0,
        .lengthfield.coding = MODE_FIXEDINT,
        .lengthfield.offset = 0,
        .lengthfield.payload = 8,
        .lengthfield.size = 4};
    cdk_handler_t handler = {
        .tcp.on_connect = _connect_cb,
        .tcp.on_read = _read_cb,
        .tcp.on_close = _close_cb,
        .tcp.on_heartbeat = _heartbeat_cb,
        .tcp.wr_timeout = 10000,
        .tcp.conn_timeout = 5000,
        .tcp.hb_interval = 5000,
        .tcp.unpacker = &unpacker};

    cdk_logger_config_t config = {
        .async = false,
    };
    cdk_logger_create(&config);
    cdk_net_dial("tcp", "127.0.0.1", "9999", &handler, 2, &conf);

    getchar();
    cdk_logger_destroy();
    return 0;
}
