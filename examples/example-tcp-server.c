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

static void _close_cb(cdk_channel_t* channel, cdk_channel_error_t error) {
    cdk_loge("connection closed, reason: %s\n", error.codestr);
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
    cdk_unpacker_t unpacker = {
        .type = UNPACKER_TYPE_LENGTHFIELD,
        .lengthfield.adj = 0,
        .lengthfield.coding = MODE_FIXEDINT,
        .lengthfield.offset = 0,
        .lengthfield.payload = 8,
        .lengthfield.size = 4};

    cdk_handler_t handler = {
        .on_accept = _accept_cb,
        .on_read = _read_cb,
        .on_close = _close_cb,
        .on_heartbeat = _heartbeat_cb,
        .rd_timeout = 10000,
        .hb_interval = 5000,
        .unpacker = &unpacker,
    };
    cdk_logger_create(NULL);
    cdk_net_listen("tcp", "0.0.0.0", "9999", &handler);

    getchar();
    cdk_net_exit();
    cdk_logger_destroy();
    return 0;
}
