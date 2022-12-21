/**
	这个模式最大的问题不是惊群，惊群再ET模式下已经解决。最大的问题是多线程饥饿。
	也就是说开8个线程，负载再1000左右，几乎只有一个线程处理，其他线程都是休眠。
*/


#include "cdk.h"
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_EVENTS_NUM 1024
#define CPUS_NUM 4

static atomic_t cnt = ATOMIC_VAR_INIT(0);
static int epfd;
static sock_t sfd;
static sock_t sfd2;

static void _netpoller_create(void) {

	epfd = epoll_create1(0);
}

static void _netpoller_destroy(void) {

	cdk_net_close(epfd);
}

static void _nonblock(sock_t s) {

	int flag = fcntl(s, F_GETFL, 0);
	if (-1 == flag) {
		abort();
	}
	fcntl(s, F_SETFL, flag | O_NONBLOCK);
}

static sock_t _tcp_accept(sock_t s) {

	sock_t c;
	do {
		c = accept(s, NULL, NULL);
	} while (c == -1 && errno == EINTR);

	if (c == -1) {
		cdk_net_close(s);
		return -1;
	}
	_nonblock(c);
	return c;
}

static void _netpoller_run(void) {
	struct epoll_event eventlst[MAX_EVENTS_NUM];//per eventlst per-thrd

	printf("[%d]epoll work thread.\n", (int)cdk_gettid());
	while (true) {
		int n = epoll_wait(epfd, (struct epoll_event*)&eventlst, MAX_EVENTS_NUM, -1);
		if (n < 0) {
			perror("epoll_wait error");
		}
		printf("[%d]epoll_wait recvice event, active fd num: %d\n", (int)cdk_gettid(), n);
		for (int i = 0; i < n; i++) {
			if (eventlst[i].data.fd == sfd) {
				sock_t cfd = _tcp_accept(sfd);
				cdk_atomic_inc(&cnt);
				printf("[%d]recvive new connection: %d\n", (int)cdk_gettid(), (int)cdk_atomic_load(&cnt));
			}
		}
	}
}

static int routine(void* p) {

	_netpoller_run();
}
int main(void) {

	_netpoller_create();

	sfd = cdk_tcp_listen("0.0.0.0", "9999");

	struct epoll_event event;
	event.data.fd = sfd;
	event.events = EPOLLIN | EPOLLET;//| EPOLLEXCLUSIVE;
	epoll_ctl(epfd, EPOLL_CTL_ADD, sfd, (struct epoll_event*)&event);

	sfd2 = cdk_tcp_listen("0.0.0.0", "8888");

	struct epoll_event event2;
	event2.data.fd = sfd2;
	event2.events = EPOLLIN | EPOLLET;// | EPOLLEXCLUSIVE; 多个epfd共享同一个socket fd时使用。
	epoll_ctl(epfd, EPOLL_CTL_ADD, sfd2, (struct epoll_event*)&event2);

	thrd_t* threads = cdk_malloc(sizeof(thrd_t) * CPUS_NUM);
	for (int i = 0; i < CPUS_NUM; i++) {
		cdk_thrd_create(&threads[i], routine, NULL);
	}
	for (int i = 0; i < CPUS_NUM; i++) {
		cdk_thrd_join(threads[i]);
	}

	return 0;
}
