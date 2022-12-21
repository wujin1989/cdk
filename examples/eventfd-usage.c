/*\

*/
#include "cdk.h"
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#define MAX_EVENTS_NUM 1024
#define CPUS_NUM 4


static int epfd, efd;

static int worker(void* p) {

	struct epoll_event eventlst[MAX_EVENTS_NUM];//per eventlst per-thrd

	printf("[%d]epoll work thread.\n", (int)cdk_gettid());
	while (true) {
		int n = epoll_wait(epfd, (struct epoll_event*)&eventlst, MAX_EVENTS_NUM, -1);
		if (n < 0) {
			perror("epoll_wait error");
		}
		printf("[%d] working.\n", (int)cdk_gettid());
		uint64_t u;
		read(efd, &u, sizeof(uint64_t));
	}
}
static int wakeup(void* p) {
	while (true) {
		uint64_t u;
		scanf("%ld", &u);
		write(efd, &u, sizeof(uint64_t));
	}
}
int main(void) {

	epfd = epoll_create1(0);
	efd = eventfd(0, EFD_NONBLOCK);

	struct epoll_event event;
	event.data.fd = efd;
	event.events = EPOLLIN | EPOLLET;//| EPOLLEXCLUSIVE;
	epoll_ctl(epfd, EPOLL_CTL_ADD, efd, (struct epoll_event*)&event);

	thrd_t* threads = cdk_malloc(sizeof(thrd_t) * CPUS_NUM);
	for (int i = 0; i < CPUS_NUM; i++) {
		cdk_thrd_create(&threads[i], worker, NULL);
	}
	thrd_t t;
	cdk_thrd_create(&t, wakeup, NULL);
	cdk_thrd_detach(t);

	for (int i = 0; i < CPUS_NUM; i++) {
		cdk_thrd_join(threads[i]);
	}

	return 0;
}
