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

#include "cdk/cdk-types.h"

extern void           _poller_create(void);
extern void           _poller_destroy(void);
extern poller_conn_t* _poller_conn_create(sock_t s, uint32_t c, poller_handler_t* h);
extern void           _poller_conn_modify(poller_conn_t* conn);
extern void           _poller_conn_destroy(poller_conn_t* conn);
extern void           _poller_poll(int epfd);
extern void           _poller_listen(const char* restrict t, const char* restrict h, const char* restrict p, poller_handler_t* handler);
extern void           _poller_dial(const char* restrict t, const char* restrict h, const char* restrict p, poller_handler_t* handler);
extern void           _poller_post_accept(poller_conn_t* conn);
extern void           _poller_post_connect(poller_conn_t* conn);
extern void           _poller_post_recv(poller_conn_t* conn);
extern void           _poller_post_send(poller_conn_t* conn);
extern void           _poller_setup_splicer(poller_conn_t* conn, splicer_profile_t* splicer);
extern int            _poller_slave(void* param);
extern void           _poller_master(void);
extern void           _poller_concurrent_slaves(int64_t num);