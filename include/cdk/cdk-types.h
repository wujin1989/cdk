/** Copyright (c) 2022, Wu Jin <wujin.developer@gmail.com>
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

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef struct poller_conn_s    poller_conn_t;
typedef struct poller_handler_s poller_handler_t;

typedef struct rb_node_s
{
	struct rb_node_s* rb_parent;
	struct rb_node_s* rb_right;
	struct rb_node_s* rb_left;
	char               rb_color;
}rb_node_t;

typedef struct rb_tree_s
{
	rb_node_t* rb_root;
}rb_tree_t;

typedef struct list_node_s {
	struct list_node_s* p;
	struct list_node_s* n;
}list_node_t;

typedef list_node_t    list_t;

typedef list_t         fifo_t;
typedef list_node_t    fifo_node_t;

typedef list_t         filo_t;
typedef list_node_t    filo_node_t;

typedef struct thrdpool_job_s {
	void          (*fn)(void*);
	void*         p;
	fifo_node_t   q_n;      /* queue node */
}thrdpool_job_t;

typedef struct ringbuf_s {
	char*       b;
	uint32_t    w;   /* write pos */
	uint32_t    r;   /* read pos */
	uint32_t    m;   /* mask */
	uint32_t    esz; /* entry size */
}ringbuf_t;

typedef enum spliter_type_e {

	SPLITER_TYPE_FIXED        = 0   ,
	SPLITER_TYPE_TEXTPLAIN    = 1   ,
	SPLITER_TYPE_BINARY       = 2   ,
	SPLITER_TYPE_USER_DEFINED = 3
}spliter_type_t;

typedef struct spliter_s {

	spliter_type_t      type;
	
	union {
		struct {
			uint32_t    len;
		}fixed;

		struct {
			char        delimiter[8];
		}textplain;

		struct {
			uint32_t  payload;   /* payload offset          */
			uint32_t  offset;    /* length field offset     */
			uint32_t  size;      /* length field size       */
			int32_t   adj;       /* length field adjustment */
			enum {
				LEN_FIELD_VARINT        ,
				LEN_FIELD_FIXEDINT
			}coding;             /* length field coding     */
		}binary;

		struct {
			void (*split)(poller_conn_t* conn);
		}userdefined;
	};
}spliter_t;

typedef struct offset_buf_s{
	void*    buf;
	uint32_t len;
	uint32_t off;
}offset_buf_t;

#if defined(__linux__) || defined(__APPLE__)

#include <stdatomic.h>
#include <netinet/in.h>
#include <pthread.h>

#if defined(__linux__)
#include <sys/types.h>
#endif

#if defined(__APPLE__)
typedef uint64_t                         tid_t;
#endif
#if defined(__linux__)
typedef pid_t                            tid_t;
#endif
typedef pthread_t                        thrd_t;
typedef pthread_once_t                   once_flag;
typedef pthread_mutex_t                  mtx_t;
typedef pthread_rwlock_t				 rwlock_t;
typedef pthread_cond_t                   cnd_t;
typedef atomic_llong                     atomic_t;
typedef int                              sock_t;

typedef struct poller_s {

	int		pfd;
	tid_t   tid;
}poller_t;

typedef struct poller_conn_s {

	poller_t			 poller;

	sock_t               fd;
	int                  cmd;
	poller_handler_t*    h;
	int                  type;
	bool                 state;
	rb_tree_t            owners;
	mtx_t                mutex;

	union {
		struct {

			offset_buf_t      ibuf;
			list_t            olist;
			spliter_t splicer;
		}tcp;

		struct {
			offset_buf_t      ibuf;
			list_t            olist;
			struct {
				struct sockaddr_storage ss;
				socklen_t               sslen;
			}peer;
		}udp;
	};
	list_node_t n;
}poller_conn_t;
#endif

#if defined(_WIN32)

#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <Windows.h>
#include <ws2ipdef.h>

typedef struct atomic_flag_s {
	_Bool v; 
}atomic_flag_t;

typedef DWORD                            tid_t;
typedef HANDLE                           thrd_t;
typedef DWORD                            pid_t;
typedef INIT_ONCE                        once_flag;
typedef CRITICAL_SECTION                 mtx_t;
typedef SRWLOCK							 rwlock_t;
typedef CONDITION_VARIABLE               cnd_t;

typedef atomic_flag_t                    atomic_flag;
typedef LONG64                           atomic_t;

typedef SOCKET                           sock_t;
typedef int                              socklen_t;
typedef SSIZE_T                          ssize_t;

typedef WSABUF iobuf_t;

typedef struct poller_conn_s {

	sock_t               fd;
	uint32_t             cmd;
	poller_handler_t*    h;
	conn_buf_t           iobuf;
	int                  iofmt;  /* user defined */
	WSAOVERLAPPED        o;
}poller_conn_t;
#endif

#define MAX_ADDRSTRLEN INET6_ADDRSTRLEN
typedef struct addrinfo_s {
	uint16_t    f;
	char        a[MAX_ADDRSTRLEN];
	uint16_t    p;
}addrinfo_t;

typedef struct thrdpool_s {
	thrd_t*       t;       /* threads */
	size_t        t_c;     /* thread count */
	fifo_t        q;
	mtx_t         t_m;     /* create thread mutex */
	mtx_t         j_m;     /* handle job mutex */
	cnd_t         j_c;
	bool          s;       /* status, idle or running */
}thrdpool_t;

typedef struct poller_handler_s {

	void (*on_accept) (poller_conn_t*);
	void (*on_connect)(poller_conn_t*);
	void (*on_read)   (poller_conn_t*, void* buf, size_t len);
	void (*on_write)  (poller_conn_t*, void* buf, size_t len);
	void (*on_close)  (poller_conn_t*);
}poller_handler_t;

