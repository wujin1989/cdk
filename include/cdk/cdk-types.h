/** Copyright (c) 2023-2033, Wu Jin <wujin.developer@gmail.com>
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

typedef struct cdk_poller_conn_s    cdk_poller_conn_t;
typedef struct cdk_poller_handler_s cdk_poller_handler_t;

typedef union cdk_rb_node_key_u {

	char*	 str;
	int8_t	 i8;
	int16_t  i16;
	int32_t  i32;
	int64_t  i64;
	uint8_t	 u8;
	uint16_t u16;
	uint32_t u32;
	uint64_t u64;
}cdk_rb_node_key_t;

typedef struct cdk_rb_node_s
{
	cdk_rb_node_key_t     rb_key;
	struct cdk_rb_node_s* rb_parent;
	struct cdk_rb_node_s* rb_right;
	struct cdk_rb_node_s* rb_left;
	char                  rb_color;
}cdk_rb_node_t;

typedef enum cdk_rb_node_keytype_e{

	RB_KEYTYPE_INT8    ,
	RB_KEYTYPE_UINT8   ,
	RB_KEYTYPE_INT16   ,
	RB_KEYTYPE_UINT16  ,
	RB_KEYTYPE_INT32   ,
	RB_KEYTYPE_UINT32  ,
	RB_KEYTYPE_INT64   ,
	RB_KEYTYPE_UINT64  ,
	RB_KEYTYPE_STR     ,
}cdk_rb_node_keytype_t;

typedef struct cdk_rb_tree_s
{
	cdk_rb_node_t*        rb_root;
	cdk_rb_node_keytype_t rb_keytype;
}cdk_rb_tree_t;

typedef struct cdk_list_node_s {
	struct cdk_list_node_s* p;
	struct cdk_list_node_s* n;
}cdk_list_node_t;

typedef cdk_list_node_t    cdk_list_t;

typedef cdk_list_node_t    cdk_queue_t;
typedef cdk_list_node_t    cdk_queue_node_t;

typedef cdk_list_node_t    cdk_stack_t;
typedef cdk_list_node_t    cdk_stack_node_t;

typedef struct cdk_thrdpool_job_s {
	void				(*fn)(void*);
	void*				p;
	cdk_queue_node_t    q_n;
}cdk_thrdpool_job_t;

typedef struct cdk_ringbuf_s {
	char*       b;
	uint32_t    w;   /* write pos */
	uint32_t    r;   /* read pos */
	uint32_t    m;   /* mask */
	uint32_t    esz; /* entry size */
}cdk_ringbuf_t;

typedef enum cdk_spliter_type_e {

	SPLITER_TYPE_FIXED        = 0   ,
	SPLITER_TYPE_TEXTPLAIN    = 1   ,
	SPLITER_TYPE_BINARY       = 2   ,
	SPLITER_TYPE_USER_DEFINED = 3
}cdk_spliter_type_t;

typedef struct cdk_spliter_s {

	cdk_spliter_type_t  type;
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
			void (*split)(cdk_poller_conn_t* conn);
		}userdefined;
	};
}cdk_spliter_t;

typedef struct cdk_offset_buf_s{
	void*    buf;
	uint32_t len;
	uint32_t off;
}cdk_offset_buf_t;

#if defined(__linux__) || defined(__APPLE__)

#include <stdatomic.h>
#include <netinet/in.h>
#include <pthread.h>

#if defined(__linux__)
#include <sys/types.h>
#endif

#if defined(__APPLE__)
typedef uint64_t                         cdk_tid_t;
#endif
#if defined(__linux__)
typedef pid_t                            cdk_tid_t;
#endif
typedef pid_t                            cdk_pid_t;
typedef pthread_t                        cdk_thrd_t;
typedef pthread_once_t                   cdk_once_flag;
typedef pthread_mutex_t                  cdk_mtx_t;
typedef pthread_rwlock_t				 cdk_rwlock_t;
typedef pthread_cond_t                   cdk_cnd_t;
typedef atomic_flag						 cdk_atomic_flag;
typedef atomic_llong                     cdk_atomic_t;
typedef int                              cdk_sock_t;

typedef struct cdk_poller_s {

	int		    pfd;
	cdk_tid_t   tid;
}cdk_poller_t;

typedef struct cdk_poller_conn_s {

	cdk_poller_t		    poller;

	cdk_sock_t				fd;
	int						cmd;
	cdk_poller_handler_t*   h;
	int						type;
	bool					state;
	cdk_rb_tree_t           owners;
	cdk_mtx_t               mutex;

	union {
		struct {

			cdk_offset_buf_t      ibuf;
			cdk_list_t            olist;
			cdk_spliter_t         splicer;
		}tcp;

		struct {
			cdk_offset_buf_t      ibuf;
			cdk_list_t            olist;
			struct {
				struct sockaddr_storage ss;
				socklen_t               sslen;
			}peer;
		}udp;
	};
	cdk_list_node_t n;
}cdk_poller_conn_t;
#endif

#if defined(_WIN32)

#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <Windows.h>
#include <ws2ipdef.h>

typedef struct cdk_atomic_flag_s {
	_Bool v; 
}cdk_atomic_flag_t;

typedef DWORD                            cdk_tid_t;
typedef HANDLE                           cdk_thrd_t;
typedef DWORD                            cdk_pid_t;
typedef INIT_ONCE                        cdk_once_flag;
typedef CRITICAL_SECTION                 cdk_mtx_t;
typedef SRWLOCK							 cdk_rwlock_t;
typedef CONDITION_VARIABLE               cdk_cnd_t;

typedef cdk_atomic_flag_t                cdk_atomic_flag;
typedef LONG64                           cdk_atomic_t;

typedef SOCKET                           cdk_sock_t;
typedef int                              socklen_t;
typedef SSIZE_T                          ssize_t;

#endif

#define MAX_ADDRSTRLEN INET6_ADDRSTRLEN
typedef struct cdk_addrinfo_s {
	uint16_t    f;
	char        a[MAX_ADDRSTRLEN];
	uint16_t    p;
}cdk_addrinfo_t;

typedef struct cdk_thrdpool_s {
	cdk_thrd_t*       t;       /* threads */
	size_t            t_c;     /* thread count */
	cdk_queue_t       q;
	cdk_mtx_t         t_m;     /* create thread mutex */
	cdk_mtx_t         j_m;     /* handle job mutex */
	cdk_cnd_t         j_c;
	bool			  s;       /* status, idle or running */
}cdk_thrdpool_t;

typedef struct cdk_poller_handler_s {

	void (*on_accept) (cdk_poller_conn_t*);
	void (*on_connect)(cdk_poller_conn_t*);
	void (*on_read)   (cdk_poller_conn_t*, void* buf, size_t len);
	void (*on_write)  (cdk_poller_conn_t*, void* buf, size_t len);
	void (*on_close)  (cdk_poller_conn_t*);
}cdk_poller_handler_t;

