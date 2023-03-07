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

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#if defined(_WIN32)

#include <WinSock2.h>
#include <Windows.h>
#include <ws2ipdef.h>

#endif

#if defined(__linux__) || defined(__APPLE__)

#include <stdatomic.h>
#include <netinet/in.h>
#include <pthread.h>

#if defined(__linux__)
#include <sys/types.h>
#endif

#endif

enum cdk_spliter_type_e {

	SPLITER_TYPE_FIXED        ,
	SPLITER_TYPE_TEXTPLAIN    ,
	SPLITER_TYPE_BINARY       ,
	SPLITER_TYPE_USER_DEFINED ,
};

enum cdk_rbtree_node_keytype_e {

	RB_KEYTYPE_INT8   ,
	RB_KEYTYPE_UINT8  ,
	RB_KEYTYPE_INT16  ,
	RB_KEYTYPE_UINT16 ,
	RB_KEYTYPE_INT32  ,
	RB_KEYTYPE_UINT32 ,
	RB_KEYTYPE_INT64  ,
	RB_KEYTYPE_UINT64 ,
	RB_KEYTYPE_STR    ,
};

typedef struct cdk_poller_conn_s         cdk_poller_conn_t;
typedef struct cdk_poller_handler_s      cdk_poller_handler_t;
typedef union  cdk_rbtree_node_key_u     cdk_rbtree_node_key_t;
typedef struct cdk_rbtree_node_s         cdk_rbtree_node_t;
typedef enum   cdk_rbtree_node_keytype_e cdk_rbtree_node_keytype_t;
typedef struct cdk_rbtree_s              cdk_rbtree_t;
typedef struct cdk_list_node_s           cdk_list_node_t;
typedef struct cdk_list_node_s           cdk_list_t;
typedef struct cdk_list_node_s           cdk_queue_t;
typedef struct cdk_list_node_s           cdk_queue_node_t;
typedef struct cdk_list_node_s           cdk_stack_t;
typedef struct cdk_list_node_s           cdk_stack_node_t;
typedef struct cdk_thrdpool_job_s        cdk_thrdpool_job_t;
typedef struct cdk_ringbuf_s             cdk_ringbuf_t;
typedef enum   cdk_spliter_type_e        cdk_spliter_type_t;
typedef struct cdk_spliter_s             cdk_spliter_t;
typedef struct cdk_offset_buf_s          cdk_offset_buf_t;
typedef struct cdk_addrinfo_s            cdk_addrinfo_t;
typedef struct cdk_thrdpool_s            cdk_thrdpool_t;

#if defined(__linux__) || defined(__APPLE__)

#if defined(__APPLE__)
typedef uint64_t             cdk_tid_t;
#endif
#if defined(__linux__)
typedef pid_t                cdk_tid_t;
#endif
typedef pid_t                cdk_pid_t;
typedef struct cdk_thrd_s    cdk_thrd_t;
typedef pthread_once_t       cdk_once_t;
typedef pthread_mutex_t      cdk_mtx_t;
typedef pthread_rwlock_t     cdk_rwlock_t;
typedef pthread_cond_t       cdk_cnd_t;
typedef atomic_flag          cdk_atomic_flag_t;
typedef atomic_llong         cdk_atomic_t;
typedef int                  cdk_sock_t;

#endif

#if defined(_WIN32)

typedef DWORD                       cdk_tid_t;
typedef struct cdk_thrd_s           cdk_thrd_t;
typedef DWORD                       cdk_pid_t;
typedef INIT_ONCE                   cdk_once_t;
typedef CRITICAL_SECTION            cdk_mtx_t;
typedef SRWLOCK                     cdk_rwlock_t;
typedef CONDITION_VARIABLE          cdk_cnd_t;
typedef struct cdk_atomic_flag_s    cdk_atomic_flag_t;
typedef LONG64                      cdk_atomic_t;
typedef SOCKET                      cdk_sock_t;
typedef int                         socklen_t;
typedef SSIZE_T                     ssize_t;

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
};

struct cdk_rbtree_node_s
{
	cdk_rbtree_node_key_t rb_key;
	struct cdk_rbtree_node_s* rb_parent;
	struct cdk_rbtree_node_s* rb_right;
	struct cdk_rbtree_node_s* rb_left;
	char rb_color;
};

struct cdk_rbtree_s
{
	cdk_rbtree_node_t* rb_root;
	cdk_rbtree_node_keytype_t rb_keytype;
};

struct cdk_list_node_s {
	struct cdk_list_node_s* p;
	struct cdk_list_node_s* n;
};

struct cdk_thrdpool_job_s {
	void (*routine)(void*);
	void* arg;
	cdk_queue_node_t n;
};

struct cdk_ringbuf_s {
	char* b;
	uint32_t w;   /* write pos */
	uint32_t r;   /* read pos */
	uint32_t m;   /* mask */
	uint32_t esz; /* entry size */
};

struct cdk_spliter_s {

	cdk_spliter_type_t type;
	union {
		struct {
			uint32_t len;
		}fixed;

		struct {
			char delimiter[8];
		}textplain;

		struct {
			uint32_t  payload;   /* payload offset          */
			uint32_t  offset;    /* length field offset     */
			uint32_t  size;      /* length field size       */
			int32_t   adj;       /* length field adjustment */
			enum {
				LEN_FIELD_VARINT,
				LEN_FIELD_FIXEDINT
			}coding;             /* length field coding     */
		}binary;

		struct {
			void (*split)(cdk_poller_conn_t* conn);
		}userdefined;
	};
};

struct cdk_offset_buf_s {
	void* buf;
	uint32_t len;
	uint32_t off;
};

#if defined(_WIN32)

struct cdk_atomic_flag_s {
	_Bool v;
};

struct cdk_thrd_s {
	HANDLE handle;
	unsigned int tid;
};

#endif

#if defined(__linux__) || defined(__APPLE__)

struct cdk_thrd_s {
	pthread_t tid;
};

#endif

struct cdk_addrinfo_s {
	uint16_t    f;
	char        a[INET6_ADDRSTRLEN];
	uint16_t    p;
};

struct cdk_thrdpool_s {

	cdk_thrd_t* thrds;
	size_t      thrdcnt;
	cdk_queue_t queue;
	cdk_mtx_t   tmtx;
	cdk_mtx_t   qmtx;
	cdk_cnd_t   qcnd;
	bool        status;
};

struct cdk_thrdpool_timed_s {
	cdk_thrd_t*  thrds;
	size_t       thrdcnt;
	cdk_rbtree_t rbtree;
	cdk_mtx_t    tmtx;
	cdk_mtx_t    rbmtx;
	cdk_cnd_t    rbcnd;
	bool         status;
};

typedef struct cdk_poller_s {

	int		    pfd;
	cdk_tid_t   tid;
}cdk_poller_t;

struct cdk_poller_conn_s {

	cdk_poller_t          poller;

	cdk_sock_t            fd;
	int                   cmd;
	cdk_poller_handler_t* h;
	int                   type;
	bool                  state;
	cdk_rbtree_t          owners;
	cdk_mtx_t             mutex;

	union {
		struct {

			cdk_offset_buf_t ibuf;
			cdk_list_t       olist;
			cdk_spliter_t    splicer;
		}tcp;

		struct {
			cdk_offset_buf_t ibuf;
			cdk_list_t       olist;
			struct {
				struct sockaddr_storage ss;
				socklen_t               sslen;
			}peer;
		}udp;
	};
	cdk_list_node_t n;
};

struct cdk_poller_handler_s {

	void (*on_accept) (cdk_poller_conn_t*);
	void (*on_connect)(cdk_poller_conn_t*);
	void (*on_read)   (cdk_poller_conn_t*, void* buf, size_t len);
	void (*on_write)  (cdk_poller_conn_t*, void* buf, size_t len);
	void (*on_close)  (cdk_poller_conn_t*);
};