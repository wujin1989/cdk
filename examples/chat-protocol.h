#include <stdint.h>

#define MAX_CHAT_MSG    1024

typedef enum _MSG_TYPE {

	TYPE_REGISTER_REQ,
	TYPE_LOGIN_REQ,
	TYPE_LOGOUT_REQ,
	TYPE_CHAT_MSG,
	TYPE_COMMON_RSP
}MSG_TYPE;

typedef struct _chat_register_req {
	char	username[64];
	char	password[64];
	char    email[64];
}chat_register_req;

typedef struct _chat_login_req {
	char	username[64];
	char	password[64];
}chat_login_req;

typedef struct _chat_logout_req {
	bool _no_used;
}chat_logout_req;

typedef struct _chat_msg {
	char    who[65];
	char	msg[MAX_CHAT_MSG];
}chat_msg;

typedef struct _chat_common_rsp {
	_Bool	status;
	char	desc[128];
}chat_common_rsp;