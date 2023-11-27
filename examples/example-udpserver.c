#include "cdk.h"

typedef struct synchronized_queue_s {
	cdk_queue_t data;
	mtx_t lock;
	cnd_t cond;
}synchronized_queue_t;

typedef cdk_queue_node_t synchronized_queue_node_t;

#define synchronized_queue_data cdk_list_data

static synchronized_queue_t mq;

typedef struct msg_s {
	cdk_channel_t* conn;
	synchronized_queue_node_t node;
	size_t len;
	char   buf[];
}msg_t;

void synchronized_queue_create(synchronized_queue_t* queue) {
	cdk_queue_init(&queue->data);
	mtx_init(&queue->lock, mtx_plain);
	cnd_init(&queue->cond);
}

void synchronized_queue_enqueue(synchronized_queue_t* queue, synchronized_queue_node_t* node) {
	mtx_lock(&queue->lock);
	cdk_queue_enqueue(&queue->data, node);
	cnd_signal(&queue->cond);
	mtx_unlock(&queue->lock);
}

bool synchronized_queue_empty(synchronized_queue_t* queue) {
	return cdk_queue_empty(&queue->data);
}

synchronized_queue_node_t* synchronized_queue_dequeue(synchronized_queue_t* queue) {
	mtx_lock(&queue->lock);
	while (synchronized_queue_empty(queue)) {
		cnd_wait(&queue->cond, &queue->lock);
	}
	cdk_queue_node_t* node = cdk_queue_dequeue(&queue->data);
	mtx_unlock(&queue->lock);
	return node;
}

void synchronized_queue_destroy(synchronized_queue_t* queue) {
	mtx_destroy(&queue->lock);
	cnd_destroy(&queue->cond);
}

int routine(void* p) {
	while (true) {
		msg_t* msg = synchronized_queue_data(synchronized_queue_dequeue(&mq), msg_t, node);
		printf("received buf: %s, len: %zu\n", msg->buf, msg->len);

		cdk_net_postsend(msg->conn, "recvive complete.", strlen("recvive complete.") + 1);

		free(msg);
		msg = NULL;
	}
	return 0;
}

static void handle_write(cdk_channel_t* conn, void* buf, size_t len) {
	cdk_net_postrecv(conn);
}
static void handle_read(cdk_channel_t* conn, void* buf, size_t len) {
	cdk_addrinfo_t ai;
	cdk_net_ntop(&conn->udp.peer.ss, &ai);

	msg_t* msg = malloc(sizeof(msg_t) + len);
	if (!msg) {
		return;
	}
	msg->len = len;
	msg->conn = conn;
	memcpy(msg->buf, buf, len);
	synchronized_queue_enqueue(&mq, &msg->node);
}

static void handle_close(cdk_channel_t* conn, char* error) {
	printf("connection closed, reason: %s\n", error);
	cdk_net_close(conn);
}
int main(void) {
	cdk_net_startup(4, NULL);
	synchronized_queue_create(&mq);

	thrd_t tid;
	thrd_create(&tid, routine, NULL);
	thrd_detach(tid);

	cdk_handler_t handler = {
		.on_read = handle_read,
		.on_write = handle_write,
		.on_close = handle_close
	};
	cdk_net_listen("udp", "0.0.0.0", "9999", &handler);
	
	cdk_net_cleanup();
	return 0;
}
