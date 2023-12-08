/** Copyright (c), Wu Jin <wujin.developer@gmail.com>
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

_Pragma("once")

#include "cdk/deprecated/c11-threads.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdatomic.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#if defined(_WIN32)
#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN

#include <WinSock2.h>
#include <Windows.h>
#include <ws2ipdef.h>
#include <WS2tcpip.h>
#include <process.h>

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"winmm.lib")
#endif

#if defined(__linux__) || defined(__APPLE__)
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <fcntl.h>

#if defined(__linux__)
#include <sys/epoll.h>
#include <sys/syscall.h>
#endif

#if defined(__APPLE__)
#include <sys/event.h>
#endif
#endif

typedef enum cdk_unpack_type_e {
	UNPACK_TYPE_FIXEDLEN    ,
	UNPACK_TYPE_DELIMITER   ,
	UNPACK_TYPE_LENGTHFIELD ,
	UNPACK_TYPE_USERDEFINED ,
}cdk_unpack_type_t;

typedef enum cdk_event_type_e{
	EVENT_TYPE_R = 1,
	EVENT_TYPE_W = 2,
	EVENT_TYPE_A = 4,
	EVENT_TYPE_C = 8,
}cdk_event_type_t;

typedef enum cdk_protocol_e {
	PROTOCOL_TCP = 1,
	PROTOCOL_UDP = 2,
}cdk_protocol_t;

#define cdk_tls_t void

typedef struct cdk_channel_s             cdk_channel_t;
typedef struct cdk_handler_s             cdk_handler_t;
typedef union  cdk_rbtree_node_key_u     cdk_rbtree_node_key_t;
typedef struct cdk_rbtree_node_s         cdk_rbtree_node_t;
typedef struct cdk_rbtree_s              cdk_rbtree_t;
typedef struct cdk_list_node_s           cdk_list_node_t;
typedef struct cdk_list_node_s           cdk_list_t;
typedef struct cdk_list_node_s           cdk_queue_t;
typedef struct cdk_list_node_s           cdk_queue_node_t;
typedef struct cdk_list_node_s           cdk_stack_t;
typedef struct cdk_list_node_s           cdk_stack_node_t;
typedef struct cdk_thrdpool_s            cdk_thrdpool_t;
typedef struct cdk_timer_job_s           cdk_timer_job_t;
typedef struct cdk_timer_s               cdk_timer_t;
typedef struct cdk_ringbuf_s             cdk_ringbuf_t;
typedef enum   cdk_unpack_type_e         cdk_unpack_type_t;
typedef struct cdk_unpack_s              cdk_unpack_t;
typedef struct cdk_offset_buf_s          cdk_offset_buf_t;
typedef struct cdk_addrinfo_s            cdk_addrinfo_t;
typedef struct cdk_poller_s              cdk_poller_t;
typedef struct cdk_event_s               cdk_event_t;
typedef struct cdk_tlsconf_s             cdk_tlsconf_t;
typedef struct cdk_sha256_s	             cdk_sha256_t;
typedef struct cdk_sha1_s	             cdk_sha1_t;
typedef struct cdk_rwlock_s              cdk_rwlock_t;
typedef struct cdk_spinlock_s            cdk_spinlock_t;

#if defined(__linux__) || defined(__APPLE__)

#if defined(__APPLE__)
typedef uint64_t             cdk_tid_t;
#endif
#if defined(__linux__)
typedef pid_t                cdk_tid_t;
#endif

#define INVALID_SOCKET       -1
typedef pid_t                cdk_pid_t;
typedef int                  cdk_sock_t;
typedef int                  cdk_pollfd_t;
#endif

#if defined(_WIN32)
typedef DWORD                       cdk_tid_t;
typedef DWORD                       cdk_pid_t;
typedef SOCKET                      cdk_sock_t;
typedef int                         socklen_t;
typedef SSIZE_T                     ssize_t;
typedef HANDLE                      cdk_pollfd_t;
#endif

union cdk_rbtree_node_key_u {
	char*    str;
	int8_t	 i8;
	int16_t  i16;
	int32_t  i32;
	int64_t  i64;
	uint8_t	 u8;
	uint16_t u16;
	uint32_t u32;
	uint64_t u64;
	void*    ptr;
};

struct cdk_rbtree_node_s
{
	cdk_rbtree_node_key_t rb_key;
	struct cdk_rbtree_node_s* rb_parent;
	struct cdk_rbtree_node_s* rb_right;
	struct cdk_rbtree_node_s* rb_left;
	char rb_color;
};

struct cdk_rwlock_s {
	atomic_int  rdcnt;
	atomic_bool wrlock;
};

struct cdk_spinlock_s {
	atomic_bool locked;
};

struct cdk_rbtree_s
{
	cdk_rbtree_node_t* rb_root;
	int(*rb_keycmp)(cdk_rbtree_node_key_t*, cdk_rbtree_node_key_t*);
};

struct cdk_list_node_s {
	struct cdk_list_node_s* p;
	struct cdk_list_node_s* n;
};

struct cdk_thrdpool_s {
	thrd_t*     thrds;
	size_t      thrdcnt;
	cdk_queue_t queue;
	mtx_t       tmtx;
	mtx_t       qmtx;
	cnd_t       qcnd;
	bool        status;
};

struct cdk_timer_job_s {
	void  (*routine)(void*);
	void*    arg;
	uint64_t birthtime;
	uint32_t expire;
	bool     repeat;
	cdk_list_node_t n;
};

struct cdk_timer_s {
	thrd_t* thrds;
	size_t thrdcnt;
	cdk_rbtree_t rbtree;
	mtx_t tmtx;
	mtx_t rbmtx;
	cnd_t rbcnd;
	bool status;
};

struct cdk_ringbuf_s {
	char* buf;
	uint32_t wpos;   /* write pos */
	uint32_t rpos;   /* read pos */
	uint32_t mask;   /* mask */
	uint32_t esz; /* entry size */
};

struct cdk_unpack_s {
	cdk_unpack_type_t type;
	union {
		struct {
			uint32_t len;
		}fixedlen;

		struct {
			char delimiter[8];
		}delimiter;

		struct {
			uint32_t  payload;   /* payload offset          */
			uint32_t  offset;    /* length field offset     */
			uint32_t  size;      /* length field size       */
			int32_t   adj;       /* length field adjustment */
			enum {
				LEN_FIELD_VARINT,
				LEN_FIELD_FIXEDINT
			}coding;             /* length field coding     */
		}lengthfield;

		struct {
			void (*unpack)(cdk_channel_t* channel);
		}userdefined;
	};
};

struct cdk_offset_buf_s {
	void* buf;
	ssize_t len;
	ssize_t off;
};

struct cdk_addrinfo_s {
	uint16_t    f;
	char        a[INET6_ADDRSTRLEN];
	uint16_t    p;
};

struct cdk_poller_s {
	cdk_pollfd_t pfd;
	thrd_t tid;
	cdk_sock_t evfds[2];
	cdk_list_t evlist;
	bool active;
	mtx_t evmtx;
	cdk_channel_t* wakeup;
	cdk_list_node_t node;
};

struct cdk_event_s {
	void (*cb)(void*);
	void* arg;
	cdk_list_node_t node;
};

struct cdk_tlsconf_s {
	const char* cafile;
	const char* capath;
	const char* crtfile;
	const char* keyfile;
	bool verifypeer;
};

struct cdk_channel_s {
	cdk_poller_t*    poller;
	cdk_sock_t       fd;
	int              events;
	cdk_handler_t*   handler;
	int              type;
	atomic_bool      closing;
	cdk_offset_buf_t rxbuf;
	cdk_list_t       txlist;
	cdk_tls_t*       tls;
	union {
		struct {
			cdk_unpack_t     unpacker;
			cdk_timer_job_t* ctimer;
		}tcp;
		struct {
			struct {
				struct sockaddr_storage ss;
				socklen_t sslen;
			}peer;
		}udp;
	};
};

struct cdk_handler_s {
	void (*on_read)   (cdk_channel_t*, void* buf, size_t len);
	void (*on_write)  (cdk_channel_t*);
	void (*on_close)  (cdk_channel_t*, char* error);

	/** the following is only used by udp. */
	void (*on_ready) (cdk_channel_t*);

	/** the following is only used by tcp. */
	void (*on_accept) (cdk_channel_t*);
	void (*on_connect)(cdk_channel_t*);
	int connect_timeout;

	/** the following is only used by tls. */
	cdk_tlsconf_t* tlsconf;
};

struct cdk_sha256_s {
	uint8_t  data[64];
	uint32_t datalen;
	uint64_t bitlen;
	uint32_t state[8];
};

struct cdk_sha1_s {
	uint32_t state[5];
	size_t count[2];
	uint8_t  buffer[64];
};
