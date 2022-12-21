
#include "cdk.h"
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_EVENTS_NUM 1024
#define CPUS_NUM 4

static atomic_t cnt = ATOMIC_VAR_INIT(0);
static int epfd;
static sock_t sfd;

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


static int routine(void* p) {

	struct epoll_event eventlst[MAX_EVENTS_NUM];//per eventlst per-thrd

	printf("[%d]epoll work thread.\n", (int)cdk_gettid());
	while (true) {
		int n = epoll_wait(epfd, (struct epoll_event*)&eventlst, MAX_EVENTS_NUM, -1);
		if (n < 0) {
			perror("epoll_wait error");
		}
		printf("[%d]epoll_wait recvice event, active fd num: %d\n", (int)cdk_gettid(), n);
		for (int i = 0; i < n; i++) {
			if (eventlst[i].events & EPOLLIN) {
				char buf[1024] = { 0 };
				recv(eventlst[i].data.fd, buf, sizeof(buf), 0);
				printf("[%d]recvive buf: %s\n", (int)cdk_gettid(), buf);
				cdk_sleep(20000);

				struct epoll_event event;
				event.data.fd = eventlst[i].data.fd;
				event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
				epoll_ctl(epfd, EPOLL_CTL_MOD, eventlst[i].data.fd, (struct epoll_event*)&event);
			}
			
		}
	}
}
int main(void) {

	epfd = epoll_create1(0);

	sfd = cdk_tcp_listen("0.0.0.0", "9999");

	for (int i = 0; i < CPUS_NUM; i++) {
		thrd_t t;
		cdk_thrd_create(&t, routine, NULL);
		cdk_thrd_detach(t);
	}
	while (true) {
		sock_t c = _tcp_accept(sfd);
		struct epoll_event event;
		event.data.fd = c;
		event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
		epoll_ctl(epfd, EPOLL_CTL_ADD, c, (struct epoll_event*)&event);
	}
	return 0;
}
