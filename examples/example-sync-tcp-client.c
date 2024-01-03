#include "cdk.h"

char ping[16] = "PING";

int main(void) {
	cdk_net_startup2();
	cdk_sock_t cli = cdk_net_dial2("tcp", "127.0.0.1", "9999");
	while (true) {
		if (cdk_net_send2(cli, ping, sizeof(ping)) <= 0) {
			break;
		}
		char rdbuf[4096] = { 0 };
		if (cdk_net_recv2(cli, rdbuf, sizeof(rdbuf)) <= 0) {
			break;
		}
		printf("recv %s\n", rdbuf);
	}
	cdk_net_close2(cli);
	cdk_net_cleanup2();
	return 0;
}
