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

#define MAX_PROCESS_EVENTS 1024

enum {

	PLATFORM_EVENT_R,
	PLATFORM_EVENT_W,
	PLATFORM_EVENT_A,
	PLATFORM_EVENT_C,
	PLATFORM_EVENT_U,
};

typedef struct cdk_pollevent_s {
	void* ptr;
}cdk_pollevent_t;

extern void platform_event_add(cdk_pollfd_t pfd, cdk_sock_t sfd, int type, void* ud);
extern void platform_event_mod(cdk_pollfd_t pfd, cdk_sock_t sfd, int type, void* ud);
extern void platform_event_del(cdk_pollfd_t pfd, cdk_sock_t sfd);
extern int  platform_event_wait(cdk_pollfd_t pfd, cdk_pollevent_t* events);