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

#include <errno.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32)
#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN

#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>
#include <mstcpip.h>
#include <process.h>
#include <ws2ipdef.h>

#pragma comment(lib, "ws2_32.lib")

#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)
#endif

#if defined(__linux__) || defined(__APPLE__)
#include <arpa/inet.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#if defined(__linux__)
#include <linux/filter.h>
#include <sys/epoll.h>
#include <sys/syscall.h>
#endif

#if defined(__APPLE__)
#include <sys/event.h>
#endif
#endif

#define cdk_tls_ssl_t void
#define cdk_tls_ctx_t void
#define cdk_tls_bio_t void

typedef struct cdk_channel_s       cdk_channel_t;
typedef enum cdk_channel_mode_e    cdk_channel_mode_t;
typedef struct cdk_handler_s       cdk_handler_t;
typedef union cdk_rbtree_key_u     cdk_rbtree_key_t;
typedef struct cdk_rbtree_node_s   cdk_rbtree_node_t;
typedef struct cdk_rbtree_s        cdk_rbtree_t;
typedef struct cdk_list_node_s     cdk_list_node_t;
typedef struct cdk_list_node_s     cdk_list_t;
typedef struct cdk_list_node_s     cdk_queue_t;
typedef struct cdk_list_node_s     cdk_queue_node_t;
typedef struct cdk_list_node_s     cdk_stack_t;
typedef struct cdk_list_node_s     cdk_stack_node_t;
typedef struct cdk_heap_node_s     cdk_heap_node_t;
typedef struct cdk_heap_s          cdk_heap_t;
typedef struct cdk_thrdpool_s      cdk_thrdpool_t;
typedef struct cdk_timer_s         cdk_timer_t;
typedef struct cdk_timermgr_s      cdk_timermgr_t;
typedef struct cdk_ringbuf_s       cdk_ringbuf_t;
typedef enum cdk_unpacker_type_e   cdk_unpacker_type_t;
typedef struct cdk_unpacker_s      cdk_unpacker_t;
typedef struct cdk_address_s       cdk_address_t;
typedef struct cdk_poller_s        cdk_poller_t;
typedef struct cdk_net_engine_s    cdk_net_engine_t;
typedef struct cdk_async_event_s   cdk_async_event_t;
typedef struct cdk_tls_conf_s      cdk_tls_conf_t;
typedef enum cdk_side_e            cdk_side_t;
typedef struct cdk_sha256_s        cdk_sha256_t;
typedef struct cdk_sha1_s          cdk_sha1_t;
typedef struct cdk_rwlock_s        cdk_rwlock_t;
typedef struct cdk_spinlock_s      cdk_spinlock_t;
typedef struct cdk_dtls_ssl_s      cdk_dtls_ssl_t;
typedef struct cdk_logger_config_s cdk_logger_config_t;
typedef enum cdk_logger_level_e    cdk_logger_level_t;
typedef enum cdk_channel_reason_e  cdk_channel_reason_t;
typedef void (*cdk_logger_cb_t)(cdk_logger_level_t level, char* msg);

#if defined(__linux__) || defined(__APPLE__)

#if defined(__APPLE__)
typedef uint64_t cdk_tid_t;
#endif
#if defined(__linux__)
typedef pid_t cdk_tid_t;
#endif

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
typedef pid_t cdk_pid_t;
typedef int   cdk_sock_t;
typedef int   cdk_pollfd_t;
#endif

#if defined(_WIN32)
typedef DWORD   cdk_tid_t;
typedef DWORD   cdk_pid_t;
typedef SOCKET  cdk_sock_t;
typedef int     socklen_t;
typedef SSIZE_T ssize_t;
typedef HANDLE  cdk_pollfd_t;
#endif

union cdk_rbtree_key_u {
    char*    str;
    int8_t   i8;
    int16_t  i16;
    int32_t  i32;
    int64_t  i64;
    uint8_t  u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
    void*    ptr;
};

struct cdk_rbtree_node_s {
    cdk_rbtree_key_t          key;
    struct cdk_rbtree_node_s* parent;
    struct cdk_rbtree_node_s* right;
    struct cdk_rbtree_node_s* left;
    char                      color;
};

struct cdk_rwlock_s {
    atomic_int  rdcnt;
    atomic_bool wrlock;
};

struct cdk_spinlock_s {
    atomic_bool locked;
};

struct cdk_rbtree_s {
    cdk_rbtree_node_t* root;
    int                (*compare)(cdk_rbtree_key_t* k1, cdk_rbtree_key_t* k2);
};

struct cdk_list_node_s {
    struct cdk_list_node_s* prev;
    struct cdk_list_node_s* next;
};

struct cdk_heap_node_s {
    struct cdk_heap_node_s* left;
    struct cdk_heap_node_s* right;
    struct cdk_heap_node_s* parent;
};

struct cdk_heap_s {
    struct cdk_heap_node_s* heap_min;
    size_t                  heap_nelts;
    /* a < b return positive that means min-heap */
    /* a > b return positive, means max-heap */
    int (*compare)(cdk_heap_node_t* a, cdk_heap_node_t* b);
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

struct cdk_timer_s {
    void            (*routine)(void* param);
    void*           param;
    size_t          birth;
    size_t          id;
    size_t          expire;
    bool            repeat;
    cdk_heap_node_t node;
};

struct cdk_timermgr_s {
    cdk_heap_t heap;
    size_t     ntimers;
};

struct cdk_ringbuf_s {
    char*    buf;
    uint32_t wpos; /* write pos */
    uint32_t rpos; /* read pos */
    uint32_t mask; /* mask */
    uint32_t esz;  /* entry size */
};

enum cdk_unpacker_type_e {
    UNPACKER_TYPE_FIXEDLEN,
    UNPACKER_TYPE_DELIMITER,
    UNPACKER_TYPE_LENGTHFIELD,
    UNPACKER_TYPE_USERDEFINED,
};

enum cdk_channel_mode_e {
    CHANNEL_MODE_ACCEPT,
    CHANNEL_MODE_CONNECT,
    CHANNEL_MODE_NORMAL,
};

struct cdk_unpacker_s {
    cdk_unpacker_type_t type;
    union {
        struct {
            uint32_t len;
        } fixedlen;

        struct {
            char delimiter[8];
        } delimiter;

        struct {
            uint32_t payload; /* payload offset          */
            uint32_t offset;  /* length field offset     */
            uint32_t size;    /* length field size       */
            int32_t  adj;     /* length field adjustment */
            enum {
                MODE_VARINT,
                MODE_FIXEDINT
            } coding; /* length field coding     */
        } lengthfield;

        struct {
            void (*unpack)(cdk_channel_t* channel);
        } userdefined;
    };
};

struct cdk_address_s {
    uint16_t family;
    char     addr[INET6_ADDRSTRLEN];
    uint16_t port;
};

struct cdk_poller_s {
    cdk_pollfd_t    pfd;
    thrd_t          tid;
    cdk_sock_t      evfds[2];
    cdk_list_t      evlist;
    mtx_t           evmtx;
    bool            active;
    cdk_list_t      chlist;
    cdk_timermgr_t  timermgr;
    cdk_list_node_t node;
};

struct cdk_net_engine_s {
    thrd_t*       thrdids;
    atomic_int    thrdcnt;
    atomic_flag   initialized;
    cdk_list_t    poller_lst;
    mtx_t         poller_mtx;
    cnd_t         poller_cnd;
    cdk_poller_t* (*poller_roundrobin)(void);
};

struct cdk_async_event_s {
    void            (*task)(void* param);
    void*           arg;
    cdk_list_node_t node;
};

enum cdk_side_e {
    SIDE_CLIENT,
    SIDE_SERVER,
};

enum cdk_logger_level_e {
    LOGGER_LEVEL_DEBUG = 0,
    LOGGER_LEVEL_INFO = 1,
    LOGGER_LEVEL_WARN = 2,
    LOGGER_LEVEL_ERROR = 3,
};

enum cdk_channel_reason_e {
    CHANNEL_REASON_USER_TRIGGERED,
    CHANNEL_REASON_WR_TIMEOUT,
    CHANNEL_REASON_RD_TIMEOUT,
    CHANNEL_REASON_CONN_TIMEOUT,
    CHANNEL_REASON_POLLER_SHUTDOWN,
    CHANNEL_REASON_SYSCALL_FAIL,
    CHANNEL_REASON_TLS_FAIL,
};

struct cdk_tls_conf_s {
    /**
     * Path to a file containing trusted CA certificates in
     * PEM format. This is used for verifying the peer's
     * certificate during TLS/SSL handshakes.
     */
    const char* cafile;
    /**
     * Path to a directory containing multiple files, each with a
     * single trusted CA certificate in PEM format. These are also
     * used for verifying the peer's certificate.
     */
    const char* capath;
    /**
     * Path to the certificate file in PEM format for this
     * TLS context. This is typically the server's
     * certificate when acting as a server or the client's
     * certificate for client authentication.
     */
    const char* crtfile;
    /**
     * Path to the private key file in PEM format that corresponds
     * to the certificate specified by crtfile. This is required
     * for the TLS context to establish secure connections.
     */
    const char* keyfile;
    /**
     * A boolean flag indicating whether the TLS context should
     * verify the peer's certificate. If set to true, the TLS
     * handshake will fail if the peer does not provide a valid
     * certificate.
     */
    bool       verifypeer;
    bool       dtls;
    cdk_side_t side;
};

struct cdk_dtls_ssl_s {
    cdk_tls_ssl_t*    dtls_ssl;
    cdk_tls_bio_t*    dtls_bio;
    cdk_rbtree_node_t node;
};

struct cdk_channel_s {
    cdk_poller_t*      poller;
    cdk_sock_t         fd;
    int                events;
    cdk_handler_t*     handler;
    int                type;
    atomic_bool        closing;
    cdk_list_t         txlist;
    cdk_channel_mode_t mode;
    cdk_side_t         side;
    cdk_timer_t*       ch_destroy_timer;
    struct {
        void*   buf;
        ssize_t len;
        ssize_t off;
    } rxbuf;
    cdk_list_node_t node;
    union {
        struct {
            bool           accepting;
            bool           connecting;
            uint64_t       latest_rd_time;
            uint64_t       latest_wr_time;
            cdk_timer_t*   conn_timer;
            cdk_timer_t*   wr_timer;
            cdk_timer_t*   rd_timer;
            cdk_timer_t*   hb_timer;
            cdk_tls_ssl_t* tls_ssl;
            cdk_tls_ctx_t* tls_ctx;
        } tcp;
        struct {
            struct {
                struct sockaddr_storage ss;
                socklen_t               sslen;
            } peer;
        } udp;
    };
};

struct cdk_handler_s {
    union {
        struct {
            void (*on_accept)(cdk_channel_t* channel);
            void (*on_connect)(cdk_channel_t* channel);
            void (*on_read)(cdk_channel_t* channel, void* buf, size_t len);
            void (*on_write)(cdk_channel_t* channel);
            void (*on_close)(
                cdk_channel_t*       channel,
                cdk_channel_reason_t code,
                const char*          reason);
            void            (*on_heartbeat)(cdk_channel_t* channel);
            int             conn_timeout;
            int             wr_timeout;
            int             rd_timeout;
            int             hb_interval;
            cdk_unpacker_t* unpacker;
        } tcp;
        struct {
            void (*on_connect)(cdk_channel_t* channel);
            void (*on_read)(cdk_channel_t* channel, void* buf, size_t len);
            void (*on_write)(cdk_channel_t* channel);
            void (*on_close)(
                cdk_channel_t*       channel,
                cdk_channel_reason_t code,
                const char*          reason);
        } udp;
    };
};

struct cdk_sha256_s {
    uint8_t  data[64];
    uint32_t datalen;
    uint64_t bitlen;
    uint32_t state[8];
};

struct cdk_sha1_s {
    uint32_t state[5];
    size_t   count[2];
    uint8_t  buffer[64];
};

struct cdk_logger_config_s {
    union {
        struct {
            char* out;
            _Bool async;
        };
        cdk_logger_cb_t callback;
    };
    cdk_logger_level_t level;
};