#include "cdk.h"
#include "chat-protocol.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

enum {

	OP_USER_REGISTER    = 1,
	OP_USER_LOGIN       = 2,
	OP_USER_LOGOUT      = 3
};

int user_selector(void) {
	int i;
	int r;
	
	cdk_logi("********************* CHAT CLIENT *********************\n");
	cdk_logi("*    1. user register.\n");
	cdk_logi("*    2. user login.\n");
	cdk_logi("*******************************************************\n");
	cdk_logi("select one from (1-2): ");

	r = scanf("%d", &i);
	if (r == EOF) {
		abort();
	}

	return i;
}

bool user_register(sock_t c) {

	chat_register_req req;
	net_msg_t*        smsg;
	net_msg_t*        rmsg;

	memset(&req, 0, sizeof(chat_register_req));
	fgetc(stdin); /* drop \n */

	cdk_logi("******************* user register *********************\n");
	cdk_logi("please input username: ");
	fgets(req.username, sizeof(req.username), stdin);
	req.username[strlen(req.username) - 1] = '\0';

	cdk_logi("please input password: ");
	fgets(req.password, sizeof(req.password), stdin);
	req.password[strlen(req.password) - 1] = '\0';

	cdk_logi("please input email: ");
	fgets(req.email, sizeof(req.email), stdin);
	req.email[strlen(req.email) - 1] = '\0';

	smsg = cdk_tcp_marshaller((char*)&req, TYPE_REGISTER_REQ, sizeof(chat_register_req));
	cdk_tcp_send(c, smsg);

	rmsg = cdk_tcp_recv(c);
	switch (rmsg->h.p_t)
	{
	case TYPE_COMMON_RSP:
	{
		chat_common_rsp rsp;
		cdk_tcp_demarshaller(rmsg, (char*)&rsp);
		return rsp.status;
	}
	default:
		break;
	}

	return false; /* unknown msg type */
}

bool user_login(sock_t c) {
	chat_login_req   req;
	net_msg_t*       smsg;
	net_msg_t*       rmsg;

	memset(&req, 0, sizeof(chat_login_req));
	fgetc(stdin); /* drop \n */

	cdk_logi("********************* user login **********************\n");
	cdk_logi("please input username: ");
	fgets(req.username, sizeof(req.username), stdin);
	req.username[strlen(req.username) - 1] = '\0';

	cdk_logi("please input password: ");
	fgets(req.password, sizeof(req.password), stdin);
	req.password[strlen(req.password) - 1] = '\0';

	smsg = cdk_tcp_marshaller((char*)&req, TYPE_LOGIN_REQ, sizeof(chat_login_req));
	cdk_tcp_send(c, smsg);

	rmsg = cdk_tcp_recv(c);
	switch (rmsg->h.p_t)
	{
	case TYPE_COMMON_RSP:
	{
		chat_common_rsp rsp;
		cdk_tcp_demarshaller(rmsg, (char*)&rsp);
		return rsp.status;
	}
	default:
		break;
	}

	return false; /* unknown msg type */
}

bool user_logout(sock_t s) {

	chat_logout_req   req;
	net_msg_t*        smsg;
	net_msg_t*        rmsg;

	smsg = cdk_tcp_marshaller((char*)&req, TYPE_LOGOUT_REQ, sizeof(chat_logout_req));
	cdk_tcp_send(s, smsg);

	rmsg = cdk_tcp_recv(s);
	switch (rmsg->h.p_t)
	{
	case TYPE_COMMON_RSP:
	{
		chat_common_rsp rsp;
		cdk_tcp_demarshaller(rmsg, (char*)&rsp);
		return rsp.status;
	}
	default:
		break;
	}

	return false;
}

void user_send(sock_t c) {

	chat_msg    cmsg;
	net_msg_t*  smsg;
	addrinfo_t  ai;

	memset(&cmsg, 0, sizeof(chat_msg));
	cdk_net_obtain_addr(c, &ai, false);

	while (true) {
		cdk_logi("[enter 'quit' to exit!] input: ");
		fgets(cmsg.msg, MAX_CHAT_MSG, stdin);
		cmsg.msg[strlen(cmsg.msg) - 1] = '\0';
		memcpy(cmsg.who, ai.a, sizeof(cmsg.who));

		if (!strncmp(cmsg.msg, "quit", strlen("quit"))) {
			if (!user_logout(c)) {
				cdk_loge("user logout failed.\n");
				continue;
			}
			cdk_logi("user logout success.\n");
			return;
		}

		smsg = cdk_tcp_marshaller((char*)&cmsg, TYPE_CHAT_MSG, sizeof(chat_msg));
		cdk_tcp_send(c, smsg);
	}
}

int main(void) {
	
	sock_t   s;
	cdk_logger_init(NULL, false);

	s = cdk_tcp_dial("127.0.0.1", "9999");

	while (true) {
		switch (user_selector())
		{
		case OP_USER_REGISTER:
		{
			if (!user_register(s)) {
				cdk_loge("*************** USER REGISTER FAILED. ***************\n");
				break;
			}
			cdk_logi("\n");
			cdk_logi("*************** USER REGISTER SUCCESS. ***************\n");
			cdk_logi("\n");
			break;
		}
		case OP_USER_LOGIN:
		{
			if (!user_login(s)) {
				cdk_loge("**************** USER LOGIN FAILED. *****************\n");
				break;
			}
			cdk_logi("\n");
			cdk_logi("**************** USER LOGIN SUCCESS. *****************\n");
			cdk_logi("\n");

			goto enter;
		}
		default:
			break;
		}
	}
enter:

	while (true) {
	
		user_send(s);
		break;
	}
	cdk_net_close(s);
	cdk_logger_destroy();

	return 0;
}
