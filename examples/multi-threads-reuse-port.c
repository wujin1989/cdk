/**
	���ģʽ������ʲô���ã�������ô��װ��
*/


#include "cdk.h"
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_EVENTS_NUM 1024
#define CPUS_NUM 4

static atomic_t cnt = ATOMIC_VAR_INIT(0);

static int _netpoller_create(void) {

	return epoll_create1(0);
}

static void _netpoller_destroy(int epfd) {

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

	if (c == -1 && (errno != EAGAIN || errno != EWOULDBLOCK)) {
		cdk_net_close(s);
		return -1;
	}
	_nonblock(c);
	return c;
}

static void _netpoller_run(sock_t sfd, int epfd) {
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

	int epfd = _netpoller_create();

	sock_t sfd = cdk_tcp_listen("0.0.0.0", "9999");

	printf("create listenfd: %d\n", sfd);
	struct epoll_event event;
	event.data.fd = sfd;
	event.events = EPOLLIN;//| EPOLLEXCLUSIVE;
	epoll_ctl(epfd, EPOLL_CTL_ADD, sfd, (struct epoll_event*)&event);

	_netpoller_run( sfd, epfd);
}
int main(void) {

	thrd_t* threads = cdk_malloc(sizeof(thrd_t) * CPUS_NUM);
	for (int i = 0; i < CPUS_NUM; i++) {
		cdk_thrd_create(&threads[i], routine, NULL);
	}
	for (int i = 0; i < CPUS_NUM; i++) {
		cdk_thrd_join(threads[i]);
	}

	return 0;
}
