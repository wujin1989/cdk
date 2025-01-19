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

#include "cdk/cdk-types.h"

#define MAX_PROCESS_EVENTS 64

typedef struct platform_pollevent_s {
	uint32_t events;
	void* ptr;
} platform_pollevent_t;

typedef enum platform_event_e {
    EVENT_RD = 1,
    EVENT_WR = 2,
} platform_event_t;

extern void platform_event_add(cdk_pollfd_t pfd, cdk_sock_t sfd, int events, void* ud);
extern void platform_event_mod(cdk_pollfd_t pfd, cdk_sock_t sfd, int events, void* ud);
extern void platform_event_del(cdk_pollfd_t pfd, cdk_sock_t sfd);
extern int  platform_event_wait(cdk_pollfd_t pfd, platform_pollevent_t* events, int timeout);