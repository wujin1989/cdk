#include "cdk.h"

char PONG[16] = "PONG";

int main(void) {
	cdk_net_startup2();
	cdk_sock_t srv = cdk_net_listen2("tcp", "0.0.0.0", "9999");
	while (true) {
		cdk_sock_t cli = cdk_net_accept2(srv);
		char rbuf[4096] = { 0 };
		if (cdk_net_recv2(cli, rbuf, sizeof(rbuf)) <= 0) {
			cdk_net_close2(cli);
			break;
		}
		printf("recv %s\n", rbuf);
		if (cdk_net_send2(cli, PONG, sizeof(PONG)) <= 0) {
			cdk_net_close2(cli);
			break;
		}
	}
	cdk_net_close2(srv);
	cdk_net_cleanup2();
	return 0;
}
