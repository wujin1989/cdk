#include "cdk.h"

typedef enum _status_t{
	LOGIN_REQ,
	LOGIN_RSP,
	LOGOUT_REQ,
	LOGOUT_RSP
}status_t;

typedef struct _userdata_t {
	status_t s;
}userdata_t;

typedef struct _login_req_t {
	char      name[64];
	uint32_t  age;
}login_req_t;

typedef struct _logout_req_t {
	char      name[64];
}logout_req_t;

typedef struct _common_rsp_t {
	char      desc[64];
}common_rsp_t;

msg_t* _marshaller(char* restrict b, int tp, int sz) {

	msg_t* m = cdk_malloc(sizeof(msg_t) + sz);

	if (!cdk_byteorder()) {
		m->h.p_s = htonl(sz);
		m->h.p_t = htonl(tp);
	}
	if (cdk_byteorder()) {
		m->h.p_s = sz;
		m->h.p_t = tp;
	}
	memcpy(m->p, b, sz);
	return m;
}
void _demarshaller(msg_t* m, char* restrict b) {

	memcpy(b, m->p, m->h.p_s);
	cdk_free(m);
}

msg_t* cdk_tcp_recv(sock_t s) {

	int      r;
	hdr_t    h;
	msg_t*   m;

	r = recv(s, (char*)&h, sizeof(hdr_t), MSG_WAITALL);
	if (r <= 0) { return NULL; }

	if (!cdk_byteorder()) {
		h.p_s = ntohl(h.p_s);
		h.p_t = ntohl(h.p_t);
	}

	m = cdk_malloc(sizeof(msg_t) + h.p_s);

	m->h = h;
	r = recv(s, m->p, h.p_s, MSG_WAITALL);
	if (r <= 0) { return NULL; }

	return m;
}
static void handle_connect(poller_conn_t* conn) {
	printf("[%d]has connected to remote...\n", (int)conn->fd);

	login_req_t login = { "aaa", 10 };
	msg_t* login_msg = _marshaller(&login, LOGIN_REQ, sizeof(login_req_t));
	cdk_net_write(conn, login_msg, sizeof(msg_t) + ntohl(login_msg->h.p_s));
	cdk_net_postsend(conn);
}

static void handle_write(poller_conn_t* conn, void* buf, size_t len) {

	msg_t* msg = buf;
	if (msg->h.p_t == LOGIN_REQ) {
		cdk_net_postrecv(conn);
	}
}

static int handle_unpack(void* p) {

	
}

static void handle_read(poller_conn_t* conn, void* buf, size_t len) {
	
}

int main(void) {

	poller_handler_t handler = {
		.on_accept  = NULL,
		.on_connect = handle_connect,
		.on_read    = handle_read,
		.on_write   = handle_write
	};
	cdk_net_dial("tcp", "127.0.0.1", "9999", &handler);

	cdk_net_poller();
	return 0;
}