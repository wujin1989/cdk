#include "cdk.h"

char PING[16] = "PING";

int main(void) {
	cdk_net_startup2();
	cdk_sock_t cli = cdk_net_dial2("tcp", "127.0.0.1", "9999");
	while (true) {
		if (cdk_net_send2(cli, PING, sizeof(PING)) <= 0) {
			break;
		}
		char rbuf[4096] = { 0 };
		if (cdk_net_recv2(cli, rbuf, sizeof(rbuf)) <= 0) {
			break;
		}
		cdk_time_sleep(500);
		printf("recv %s\n", rbuf);
	}
	cdk_net_close2(cli);
	cdk_net_cleanup2();
	return 0;
}
