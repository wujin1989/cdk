#include "cdk.h"

char pong[16] = "PONG";

int main(void) {
	cdk_net_startup2();
	cdk_sock_t srv = cdk_net_listen2("tcp", "0.0.0.0", "9999");
	while (true) {
		cdk_sock_t cli = cdk_net_accept2(srv);
		char rdbuf[4096] = { 0 };
		if (cdk_net_recv2(cli, rdbuf, sizeof(rdbuf)) <= 0) {
			cdk_net_close2(cli);
			break;
		}
		printf("recv %s\n", rdbuf);
		if (cdk_net_send2(cli, pong, sizeof(pong)) <= 0) {
			cdk_net_close2(cli);
			break;
		}
	}
	cdk_net_close2(srv);
	cdk_net_cleanup2();
	return 0;
}
