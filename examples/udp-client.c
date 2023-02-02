#include "cdk.h"
#include <string.h>

char str[4096] = "hello";

thrd_t wthrd;

int send_thread(void* p) {
	poller_conn_t* conn = p;

	while (true) {
		cdk_net_postsend(conn, str, sizeof(str));
		cdk_sleep(40);
		break;
	}
	return 0;
}
static void handle_connect(poller_conn_t* conn) {
	printf("tid[%d],[%d]has connected to remote...\n", (int)cdk_gettid(), (int)conn->fd);
	

	cdk_thrd_create(&wthrd, send_thread, conn);
	cdk_thrd_detach(wthrd);
}

static void handle_write(poller_conn_t* conn, void* buf, size_t len) {
	printf("%s  ----> send ok.\n", (char*)buf);

	//cdk_net_postsend(conn, str, strlen(str) + 1);
}

static void handle_read(poller_conn_t* conn, void* buf, size_t len) {
	
}
static void handle_close(poller_conn_t* conn) {

	cdk_net_close(conn);
}
int main(void) {

	poller_handler_t handler = {
		.on_accept = NULL,
		.on_connect = handle_connect,
		.on_read = handle_read,
		.on_write = handle_write,
		.on_close = handle_close
	};
	cdk_net_dial("udp", "127.0.0.1", "9999", &handler);

	cdk_net_poller();
	return 0;
}