#include "cdk.h"

typedef struct msg_s {
	cdk_channel_t* conn;
	cdk_queue_node_t node;
	size_t len;
	char   buf[];
}msg_t;

cdk_queue_t mq;

int routine(void* p) {
	
	while (true) {
		if (!cdk_queue_empty(&mq)) {
			msg_t* msg = cdk_queue_data(cdk_queue_dequeue(&mq), msg_t, node);
			printf("received buf: %s, len: %zu\n", msg->buf, msg->len);

			cdk_net_channelsend(msg->conn, "recvive complete.", strlen("recvive complete.") + 1);

			free(msg);
			msg = NULL;
		}
		else {
			cdk_time_sleep(1000);
		}
	}
	return 0;
}

static void handle_write(cdk_channel_t* conn, void* buf, size_t len) {
	cdk_net_channelrecv(conn);
}
static void handle_read(cdk_channel_t* conn, void* buf, size_t len) {

	cdk_addrinfo_t ai;
	cdk_net_ntop(&conn->udp.peer.ss, &ai);

	msg_t* msg = malloc(sizeof(msg_t) + len);
	if (msg) {
		msg->len = len;
		msg->conn = conn;
		memcpy(msg->buf, buf, len);
		cdk_queue_enqueue(&mq, &msg->node);
	}
}

static void handle_close(cdk_channel_t* conn, char* error) {
	printf("connection closed, reason: %s\n", error);
	cdk_net_channelclose(conn);
}
int main(void) {

	cdk_tlsconf_t conf = {
		.cafile = "",
		.capath = "",
		.crtfile = "",
		.keyfile = ""
	};
	cdk_net_startup(4, &conf, NULL);
	cdk_queue_init(&mq);

	thrd_t tid;
	thrd_create(&tid, routine, NULL);
	thrd_detach(tid);

	cdk_handler_t handler = {
		.on_read = handle_read,
		.on_write = handle_write,
		.on_close = handle_close
	};
	cdk_net_listen("udp", "0.0.0.0", "9999", &handler);
	
	cdk_net_poll();
	cdk_net_cleanup();
	return 0;
}
