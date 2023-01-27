#include "cdk.h"
#include <string.h>

typedef struct _net_msg_hdr_t {

	uint32_t      p_s;   /* payload size */
	uint32_t      p_t;   /* payload type */
}net_msg_hdr_t;

typedef struct _net_msg_t {

	net_msg_hdr_t      h;
	char               p[];
}net_msg_t;

static void handle_accept(poller_conn_t* conn) {
	printf("[%d]new connection coming...\n", (int)conn->fd);

	splicer_profile_t profile1 = {
		.type = SPLICE_TYPE_FIXED,
		.fixed.len = 6
	};
	splicer_profile_t profile2 = {
		.type = SPLICE_TYPE_TEXTPLAIN,
		.textplain.delimiter = "\r\n\r\n"
	};
	cdk_net_setup_splicer(conn, &profile2);
	cdk_net_postrecv(conn);
}

static void handle_write(poller_conn_t* conn, void* buf, size_t len) {

	printf("recv write complete, %s\n", (char*)buf);
	cdk_net_postrecv(conn);
}
static void handle_read(poller_conn_t* conn, void* buf, size_t len) {

	char str[4096] = {0};
	memcpy(str, buf, len);
	printf("recv buf: %s\n", str);
	cdk_net_postsend(conn, str, strlen(str) + 1);
	
	/*char* outbuf = "hello world\n";
	cdk_net_write(conn, outbuf, strlen(outbuf)+1);
	cdk_net_postsend(conn);*/
}

int main(void) {

	poller_handler_t handler = {
		.on_accept  = handle_accept,
		.on_connect = NULL,
		.on_read    = handle_read,
		.on_write   = handle_write
	};
	cdk_net_listen("tcp", "0.0.0.0", "9999", &handler);
	
	cdk_net_poller();
	return 0;
}