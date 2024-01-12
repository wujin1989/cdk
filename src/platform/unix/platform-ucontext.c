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

#include "platform/platform-ucontext.h"

void platform_makecontext(cdk_ucontext_t* ucp, void (*func)(void*), void* arg) {
	uint64_t* sp;

	sp = (uint64_t*)((uintptr_t)ucp->uc_stack.sp + ucp->uc_stack.sz);
	sp = (uint64_t*)((uintptr_t)sp & -16L);
	ucp->uc_mctx.rip = (uintptr_t)func;
	ucp->uc_mctx.rsp = (uintptr_t)sp;
	ucp->uc_mctx.rdi = (uintptr_t)arg;
}

void platform_swapcontext(cdk_ucontext_t* oucp, const cdk_ucontext_t* ucp) {
	platform_getcontext(oucp);
	platform_setcontext(ucp);
}