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

#ifndef __CDK_NET_H__
#define __CDK_NET_H__

#include "cdk-types.h"
/* //////////////////////////////////////////common////////////////////////////////////////////// */

/**
 *  close socket.
 *
 *  @param s  [in].
 *  @return N/A.
 */
extern void cdk_net_close(sock_t s);

/**
 *  set the socket's receive buffer size to the specified v value.
 *
 *  @param s [in].
 *  @param v [in] recvive buffer size.
 *  @return N/A.
 */
extern void cdk_net_rbuf(sock_t s, int v);

/**
 *  set the socket's send buffer size to the specified v value.
 *
 *  @param s [in].
 *  @param v [in] send buffer size.
 *  @return N/A.
 */
extern void cdk_net_sbuf(sock_t s, int v);

/**
 *  convert network address format to presentable address.
 *
 *  @param ss [in] ipv4 or ipv6 address.
 *  @param ai [in/out].
 *  @return N/A.
 */
extern void cdk_net_inet_ntop(struct sockaddr_storage* ss, addrinfo_t* ai);

/**
 *  convert presentable address format to network address.
 *
 *  @param ai [in].
 *  @param ss [in/out]  ipv4 or ipv6 address.
 *  @return N/A.
 */
extern void cdk_net_inet_pton(addrinfo_t* ai, struct sockaddr_storage* ss);

/**
 *  retrive local or peer address based on connected socket fd.
 *
 *  @param s  [in] .
 *  @param ai [in/out].
 *  @param p  [in] true means obtaining peer address, false means obtaining local address.
 *  @return N/A.
 */
extern void cdk_net_obtain_addr(sock_t s, addrinfo_t* ai, bool p);

/**
 *  retrive address family by socket fd, AF_INET and AF_INET6 supported.
 *
 *  @param s  [in].
 *  @return address family.
 */
extern int cdk_net_af(sock_t s);

/* //////////////////////////////////////////tcp////////////////////////////////////////////// */

/**
 *  create a tcp blocking socket and listen.
 *
 *  @param h [in] host.
 *  @param p [in] port.
 *  @return tcp socket fd.
 */
extern void cdk_net_listen(const char* restrict t, const char* restrict h, const char* restrict p, poller_handler_t* handler);

extern void cdk_net_poller(void);

extern void post_send(poller_conn_t* conn);
extern void post_recv(poller_conn_t* conn);

/**
 *  create a tcp blocking socket and connect.
 *
 *  @param h [in] host.
 *  @param p [in] port.
 *  @return connection fd.
 */
extern sock_t cdk_tcp_dial(const char* restrict h, const char* restrict p);

/**
 *  marshalling a net_msg_t.
 *
 *  @param b  [in] payload.
 *  @param tp [in] payload type.
 *  @param sz [in] payload size.
 *  @return net_msg_t pointer.
 */
extern net_msg_t* cdk_tcp_marshaller(char* restrict b, int tp, int sz);

/**
 *  demarshalling a net_msg_t.
 *
 *  @param m  [in].
 *  @param b  [in] payload.
 *  @return N/A.
 */
extern void cdk_tcp_demarshaller(net_msg_t* m, char* restrict b);

/**
 *  send a net_msg_t.
 *  note: this function is mtu safe.
 *
 *  @param s [in].
 *  @param m [in].
 *  @return sended bytes.
 */
extern int cdk_tcp_send(sock_t s, net_msg_t* restrict m);

/**
 *  recv a net_msg_t.
 *
 *  @param s [in].
 *  @return net_msg_t pointer.
 */
extern net_msg_t* cdk_tcp_recv(sock_t s);

/* //////////////////////////////////////////udp////////////////////////////////////////////// */

/**
 *  create a udp blocking socket and bind.
 *
 *  @param h [in] host.
 *  @param p [in] port.
 *  @return udp socket fd.
 */
extern sock_t cdk_udp_listen(const char* restrict h, const char* restrict p);

/**
 *  create a udp blocking socket and connect.
 *
 *  @param h [in] host.
 *  @param p [in] port.
 *  @return udp socket fd.
 */
extern sock_t cdk_udp_dial(const char* restrict h, const char* restrict p);

/**
 *  send a udp msg.
 *
 *  @param s  [in].
 *  @param b  [in] buffer.
 *  @param sz [in] buffer size.
 *  @param ss [in] peer address(support ipv4 or ipv6).
 *  @param l  [in] peer address length.
 *  @return sended bytes.
 */
extern int cdk_udp_send(sock_t s, char* restrict b, int sz, struct sockaddr_storage* restrict ss, socklen_t l);

/**
 *  recv a udp msg.
 *
 *  @param s  [in].
 *  @param b  [in] buffer.
 *  @param sz [in] buffer size.
 *  @param ss [in] peer address(support ipv4 or ipv6).
 *  @param lp [in] peer address length.
 *  @return received bytes.
 */
extern int cdk_udp_recv(sock_t s, char* restrict b, int sz, struct sockaddr_storage* restrict ss, socklen_t* restrict lp);

#endif /* __CDK_NET_H__ */
