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
#ifndef __WIN_NET_H__
#define __WIN_NET_H__

#include "cdk/cdk-types.h"
/* common */
extern void   _cdk_net_rtimeo(sock_t s, int t);
extern void   _cdk_net_stimeo(sock_t s, int t);
extern int    _cdk_net_af(sock_t s);
extern void   _cdk_net_close(sock_t s);

/* tcp */
extern sock_t _cdk_tcp_listen(const char* restrict h, const char* restrict p);
extern sock_t _cdk_tcp_dial(const char* restrict h, const char* restrict p);

/* udp */
extern sock_t _cdk_udp_listen(const char* restrict h, const char* restrict p);
extern sock_t _cdk_udp_dial(const char* restrict h, const char* restrict p);

#endif /* __WIN_NET_H__ */

