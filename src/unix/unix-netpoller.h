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
#ifndef __UNIX_NETPOLLER_H__
#define __UNIX_NETPOLLER_H__

#include "cdk/cdk-types.h"

typedef enum _netpoller_cmd_t {

	_NETPOLLER_CMD_R     ,
	_NETPOLLER_CMD_W     ,
	_NETPOLLER_CMD_A     ,
	_NETPOLLER_CMD_C     ,
	_NETPOLLER_CMD_U
}netpoller_cmd_t;

typedef struct _netpoller_handler_t {

	void (*on_accept) (sock_t s);
	void (*on_connect)(sock_t s);
	void (*on_read)   (sock_t s, void* buf, size_t sz);
	void (*on_write)  (sock_t s);
}netpoller_handler_t;

typedef struct _netpoller_ctx_t {

	sock_t               fd;
	netpoller_cmd_t      cmd;
	netpoller_handler_t* h;
}netpoller_ctx_t;

#endif /* __UNIX_NETPOLLER_H__ */
