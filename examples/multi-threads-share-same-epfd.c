/**
	这个模式最大的问题不是惊群，惊群再ET模式下已经解决。最大的问题是多线程饥饿。
	也就是说开8个线程，负载再1000左右，几乎只有一个线程处理，其他线程都是休眠。
*/


#include "cdk.h"
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>

#define MAX_EVENTS_NUM 1024
#define CPUS_NUM 4
#define TCPv4_MSS       536
#define TCPv6_MSS       1220

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


static void _net_nonblock(sock_t s) {

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
        return -1;
    }
    _net_nonblock(c);
    return c;
}

static void _tcp_nodelay(sock_t s, bool on) {

    int v;
    if (on) { v = 1; }
    else { v = 0; }
    setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (const void*)&v, sizeof(v));
}

static void _net_reuse_addr(sock_t s) {

    int r;
    int on = 1;
    r = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const void*)&on, sizeof(on));
    if (r < 0) { abort(); }
}


static void _tcp_keepalive(sock_t s) {

    int r;
    int on = 1;
    int d = 60;
    int i = 1;  /* 1 second; same as default on win32 */
    int c = 10; /* 10 retries; same as hardcoded on win32 since vista */

    r = setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (const void*)&on, sizeof(on));
    if (r < 0) { abort(); }
    r = setsockopt(s, IPPROTO_TCP, TCP_KEEPIDLE, (const void*)&d, sizeof(d));
    if (r < 0) { abort(); }
    r = setsockopt(s, IPPROTO_TCP, TCP_KEEPINTVL, (const void*)&i, sizeof(i));
    if (r < 0) { abort(); }
    r = setsockopt(s, IPPROTO_TCP, TCP_KEEPCNT, (const void*)&c, sizeof(c));
    if (r < 0) { abort(); }
}
static int _net_af(sock_t s) {

    int       d;
    socklen_t l;

    l = sizeof(int);
    getsockopt(s, SOL_SOCKET, SO_DOMAIN, (char*)&d, (socklen_t*)&l);

    return d;
}
static void _tcp_maxseg(sock_t s) {

    int    v;
    int    af;
    int    r;

    af = _net_af(s);
    v = af == AF_INET ? TCPv4_MSS : TCPv6_MSS;

    r = setsockopt(s, IPPROTO_TCP, TCP_MAXSEG, (char*)&v, sizeof(int));
    if (r < 0) { abort(); }
}


static void _net_reuse_port(sock_t s) {

    int r;
    int on = 1;
    r = setsockopt(s, SOL_SOCKET, SO_REUSEPORT, (const void*)&on, sizeof(on));
    if (r < 0) { abort(); }
}

static sock_t _net_listen(const char* restrict h, const char* restrict p, int t) {
    int                 r;
    int                 s;
    struct addrinfo     hints;
    struct addrinfo* res;
    struct addrinfo* rp;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = t;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    r = getaddrinfo(h, p, &hints, &res);
    if (r != 0) { return -1; }

    for (rp = res; rp != NULL; rp = rp->ai_next) {

        s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (s == -1) { continue; }

        _net_reuse_addr(s);
        _net_reuse_port(s);

        if (bind(s, rp->ai_addr, rp->ai_addrlen) == -1) { close(s); continue; }
        /**
         * these options inherited by connection-socket.
         */
        if (t == SOCK_STREAM) {
            if (listen(s, SOMAXCONN) == -1) {
                close(s);
                continue;
            }
            _tcp_maxseg(s);
            _tcp_nodelay(s, true);
            _tcp_keepalive(s);
        }
        /**
         * this option not inherited by connection-socket.
         */
        _net_nonblock(s);
        break;
    }
    if (rp == NULL) { return -1; }
    freeaddrinfo(res);
    return s;
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
                struct epoll_event event;
                event.data.fd = cfd;
                event.events = EPOLLIN | EPOLLET;
                epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, (struct epoll_event*)&event);
				printf("[%d]recvive new connection: %d\n", cfd);
			}
            if (eventlst[i].events & EPOLLIN) {
                printf("[%d]epollin\n", eventlst[i].data.fd);
            }
            if (eventlst[i].events & EPOLLOUT) {
                printf("epollout\n");
            }
		}
	}
}

static int routine(void* p) {

	_netpoller_run();
}
int main(void) {

	_netpoller_create();

	sfd = _net_listen("0.0.0.0", "9999", SOCK_STREAM);

	struct epoll_event event;
	event.data.fd = sfd;
	event.events = EPOLLIN | EPOLLET;
	epoll_ctl(epfd, EPOLL_CTL_ADD, sfd, (struct epoll_event*)&event);


	thrd_t* threads = cdk_malloc(sizeof(thrd_t) * CPUS_NUM);
	for (int i = 0; i < CPUS_NUM; i++) {
		cdk_thrd_create(&threads[i], routine, NULL);
	}
	for (int i = 0; i < CPUS_NUM; i++) {
		cdk_thrd_join(threads[i]);
	}

	return 0;
}
