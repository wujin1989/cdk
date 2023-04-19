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

#include "platform/platform-socket.h"
#include "platform/platform-utils.h"
#include "platform/platform-poller.h"
#include "platform/platform-event.h"
#include "cdk/container/cdk-list.h"
#include "net/cdk-channel.h"
#include "net/cdk-unpack.h"
#include "cdk/net/cdk-net.h"

extern cdk_poller_t* _poller_roundrobin(void);

static char* __format_lasterror(DWORD error) {
    char* buffer = NULL;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&buffer, 0, NULL);
    return buffer;
}

void platform_channel_recv(cdk_channel_t* channel) {
    ssize_t n;

    if (channel->type == SOCK_STREAM) {
        n = platform_socket_recv(channel->fd, (char*)(channel->tcp.rxbuf.buf) + channel->tcp.rxbuf.off, MAX_IOBUF_SIZE);
        /**
         * windows provides the WSAECONNRESET to detect a peer disconnection,
         * so it is not necessary to rely on receiving 0 from recv to determine that the connection has been closed.
         */
        if (n == -1) {
            if (WSAGetLastError() != WSAEWOULDBLOCK) {
                channel->handler->on_close(channel, __format_lasterror(WSAGetLastError()));
            }
            return;
        }
        channel->tcp.rxbuf.off += n;
        cdk_unpack(channel);
    }
    if (channel->type == SOCK_DGRAM) {

        channel->udp.peer.sslen = sizeof(struct sockaddr_storage);
        n = platform_socket_recvfrom(channel->fd, channel->udp.rxbuf.buf, MAX_IOBUF_SIZE, &channel->udp.peer.ss, &channel->udp.peer.sslen);

        if (n == -1) {
            /**
             * to be compatible with the semantics of linux, WSAECONNRESET is filtered out. 
             */
            if (WSAGetLastError() != WSAEWOULDBLOCK && WSAGetLastError() != WSAECONNRESET) {
                channel->handler->on_close(channel, __format_lasterror(WSAGetLastError()));
            }
            return;
        }
        channel->handler->on_read(channel, channel->udp.rxbuf.buf, n);
    }
    return;
}

void platform_channel_send(cdk_channel_t* channel)
{
    if (channel->type == SOCK_STREAM) {
        while (!cdk_list_empty(&(channel->tcp.txlist))) {
            cdk_txlist_node_t* e = cdk_list_data(cdk_list_head(&(channel->tcp.txlist)), cdk_txlist_node_t, n);
            
            while (e->off < e->len) {
                ssize_t n = platform_socket_send(channel->fd, e->buf + e->off, (int)(e->len - e->off));
                if (n == -1) {
                    if (WSAGetLastError() != WSAEWOULDBLOCK) {
                        channel->handler->on_close(channel, __format_lasterror(WSAGetLastError()));
                    }
                    return;
                }
                e->off += n;
            }
            channel->handler->on_write(channel, e->buf, e->len);
            cdk_list_remove(&(e->n));
            free(e);
            e = NULL;
        }
    }
    if (channel->type == SOCK_DGRAM) {
        while (!cdk_list_empty(&(channel->udp.txlist))) {
            cdk_txlist_node_t* e = cdk_list_data(cdk_list_head(&(channel->udp.txlist)), cdk_txlist_node_t, n);

            ssize_t n = platform_socket_sendto(channel->fd, e->buf, (int)e->len, &(channel->udp.peer.ss), channel->udp.peer.sslen);
            if (n == -1) {
                if (WSAGetLastError() != WSAEWOULDBLOCK) {
                    channel->handler->on_close(channel, __format_lasterror(WSAGetLastError()));
                }
                return;
            }
            channel->handler->on_write(channel, e->buf, e->len);
            cdk_list_remove(&(e->n));
            free(e);
            e = NULL;
        }
    }
    return;
}

void platform_channel_accept(cdk_channel_t* channel) {
    cdk_sock_t cli = platform_socket_accept(channel->fd);
    if (cli == -1) {
        if (WSAGetLastError() != WSAEWOULDBLOCK) {
            channel->handler->on_close(channel, __format_lasterror(WSAGetLastError()));
        }
        return;
    }
    cdk_channel_t* newchannel = cdk_channel_create(_poller_roundrobin(), cli, PLATFORM_EVENT_R, channel->handler);
    channel->handler->on_accept(newchannel);
    return;
}

void platform_channel_connect(cdk_channel_t* channel) {
    int err;
    socklen_t len;
    len = sizeof(int);
  
    getsockopt(channel->fd, SOL_SOCKET, SO_ERROR, (char*)&err, &len);
    if (err) {
        channel->handler->on_close(channel, __format_lasterror(err));
    }
    else {
        channel->tcp.connected = true;
        channel->handler->on_connect(channel);
    }
}