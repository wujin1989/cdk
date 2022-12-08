#include "cdk.h"
#include "chat-protocol.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

once_flag once;

list_t    register_users;
list_t    login_users;

typedef struct _register_user {
	list_node_t    n;
	char           username[64];
	char           password[64];
	char           email[64];
}register_user;

typedef struct _login_user {
	list_node_t    n;
	char           username[64];
	char           password[64];
	sock_t         s;
	addrinfo_t     ai;
}login_user;

void handle_register(sock_t s, net_msg_t* rmsg, list_t* lst) {

	chat_register_req  req;
	chat_common_rsp    rsp;
	net_msg_t*         smsg;
	register_user*     new;

	cdk_tcp_demarshaller(rmsg, (char*)&req);

	memset(&rsp, 0, sizeof(chat_common_rsp));

	/* jugde user if registered */
	for (list_node_t* n = cdk_list_head(lst); n != cdk_list_sentinel(lst); n = cdk_list_next(n)) {

		register_user* u = cdk_list_data(n, register_user, n);
		if (!strcmp(u->username, req.username)) {
			rsp.status = false;
			memcpy(rsp.desc, "user had register", strlen("user had register"));

			smsg = cdk_tcp_marshaller((char*)&rsp, TYPE_COMMON_RSP, sizeof(chat_common_rsp));
			cdk_tcp_send(s, smsg);
			return;
		}
	}
	new = cdk_malloc(sizeof(register_user));
	memcpy(new->username, req.username, sizeof(req.username));
	memcpy(new->password, req.password, sizeof(req.password));
	memcpy(new->email, req.email, sizeof(req.email));
	cdk_list_init_node(&new->n);

	cdk_list_insert_tail(&register_users, &new->n);
	
	rsp.status = true;
	memcpy(rsp.desc, "register success", strlen("register success"));

	smsg = cdk_tcp_marshaller((char*)&rsp, TYPE_COMMON_RSP, sizeof(chat_common_rsp));
	cdk_tcp_send(s, smsg);
}

void handle_login(sock_t s, net_msg_t* rmsg, list_t* lst) {

	chat_login_req  req;
	chat_common_rsp rsp;
	net_msg_t*      smsg;
	login_user*     new;
	addrinfo_t      ai;

	cdk_tcp_demarshaller(rmsg, (char*)&req);

	memset(&rsp, 0, sizeof(chat_common_rsp));

	cdk_net_obtain_addr(s, &ai, true);

	/* vertify username and password */
	for (list_node_t* n = cdk_list_head(&register_users); n != cdk_list_sentinel(&register_users); n = cdk_list_next(n)) {

		register_user* u = cdk_list_data(n, register_user, n);
		if (strcmp(u->username, req.username) || strcmp(u->password, req.password)) {
			continue;
		}
	}

	/* jugde user if logined */
	for (list_node_t* n = cdk_list_head(lst); n != cdk_list_sentinel(lst); n = cdk_list_next(n)) {

		login_user* u = cdk_list_data(n, login_user, n);
		if (!strcmp(u->username, req.username)) {
			rsp.status = false;
			memcpy(rsp.desc, "user had logined", strlen("user had logined"));

			smsg = cdk_tcp_marshaller((char*)&rsp, TYPE_COMMON_RSP, sizeof(chat_common_rsp));
			cdk_tcp_send(s, smsg);
			return;
		}
	}
	new = cdk_malloc(sizeof(login_user));
	memcpy(new->username, req.username, sizeof(req.username));
	memcpy(new->password, req.password, sizeof(req.password));
	memcpy(&new->ai, &ai, sizeof(addrinfo_t));
	new->s = s;
	cdk_list_init_node(&new->n);
	
	cdk_list_insert_tail(&login_users, &new->n);

	rsp.status = true;
	memcpy(rsp.desc, "login success", strlen("login success"));

	smsg = cdk_tcp_marshaller((char*)&rsp, TYPE_COMMON_RSP, sizeof(chat_common_rsp));
	cdk_tcp_send(s, smsg);
}

void handle_logout(sock_t s, net_msg_t* rmsg, list_t* lst) {

	chat_logout_req  req;
	chat_common_rsp  rsp;
	net_msg_t*       smsg;

	cdk_tcp_demarshaller(rmsg, (char*)&req);

	memset(&rsp, 0, sizeof(chat_common_rsp));
	for (list_node_t* n = cdk_list_head(&login_users); n != cdk_list_sentinel(&login_users); ) {

		login_user* u = cdk_list_data(n, login_user, n);
		n = cdk_list_next(n);

		if (u->s == s) {
			cdk_list_remove(&u->n);
			cdk_free(u);

			rsp.status = true;
			memcpy(rsp.desc, "logout success", strlen("logout success"));

			smsg = cdk_tcp_marshaller((char*)&rsp, TYPE_COMMON_RSP, sizeof(chat_common_rsp));
			cdk_tcp_send(s, smsg);
			return;
		}
	}
	rsp.status = false;
	memcpy(rsp.desc, "logout failed", strlen("logout failed"));

	smsg = cdk_tcp_marshaller((char*)&rsp, TYPE_COMMON_RSP, sizeof(chat_common_rsp));
	cdk_tcp_send(s, smsg);
}

void handle_msg(sock_t s, net_msg_t* rmsg) {

	chat_msg cmsg;

	cdk_tcp_demarshaller(rmsg, (char*)&cmsg);
	printf("%s: %s\n", cmsg.who, cmsg.msg);
}

int monitor(void* p) {

	while (true) {
		printf("******************************** MONITOR ***************************************\n");
		printf("\n");
		printf("register_users:\n");
		printf("\n");

		for (list_node_t* n = cdk_list_head(&register_users); n != cdk_list_sentinel(&register_users); n = cdk_list_next(n)) {

			register_user* u = cdk_list_data(n, register_user, n);
			printf("\tusername: %s, password: %s, email: %s\n", u->username, u->password, u->email);
		}
		printf("\n");
		printf("login_users:\n");
		printf("\n");
		for (list_node_t* n = cdk_list_head(&login_users); n != cdk_list_sentinel(&login_users); n = cdk_list_next(n)) {

			login_user* u = cdk_list_data(n, login_user, n);
			printf("\tusername: %s, password: %s, addrs: %s:%d\n", u->username, u->password, u->ai.a, u->ai.p);
		}
		cdk_sleep(10000);
	}
	return 0;
}

static void handle_once(void) {

	cdk_list_create(&register_users);
	cdk_list_create(&login_users);

	/* enable monitor */
	//thrd_t t;
	//cdk_thrd_create(&t, monitor, NULL);
	//cdk_thrd_detach(t);
}

int routine(void* p) {
	sock_t* sp = p;
	sock_t s   = *sp;

	cdk_thrd_once(&once, handle_once);

	while (true) {
		net_msg_t* msg = cdk_tcp_recv(s);

		/* connection closed */
		if (!msg) {
			for (list_node_t* n = cdk_list_head(&login_users); n != cdk_list_sentinel(&login_users); ) {

				login_user* u = cdk_list_data(n, login_user, n);
				n = cdk_list_next(n);

				if (u->s == s) {
					cdk_list_remove(&u->n);
					cdk_free(u);
				}
			}
			cdk_net_close(s);
			return -1;
		}
		switch (msg->h.p_t)
		{
		case TYPE_REGISTER_REQ:
		{
			handle_register(s, msg, &register_users);
			break;
		}
		case TYPE_LOGIN_REQ:
		{
			handle_login(s, msg, &login_users);
			break;
		}
		case TYPE_LOGOUT_REQ:
		{
			handle_logout(s, msg, &login_users);
			cdk_net_close(s);
			return 0;
		}
		case TYPE_CHAT_MSG:
		{
			handle_msg(s, msg);
		}
		default:
			break;
		}
	}
	return 0;
}

int main(void) {
	sock_t c, s;

	cdk_logger_init(NULL, true);

	printf("******************************** CHAT ROOM ***************************************\n");

	s = cdk_tcp_listen("0.0.0.0", "9999");
	while (true) {
		//c = cdk_tcp_accept(s);

		thrd_t t;
		if (!cdk_thrd_create(&t, routine, &c)) { break; }
		cdk_thrd_detach(t);
	}
	cdk_net_close(c);
	cdk_net_close(s);

	cdk_logger_destroy();
	return 0;
}