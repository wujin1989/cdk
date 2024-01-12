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

 .globl platform_getcontext
platform_getcontext:
	movq	%rdi,		0(%rcx)
	movq	%rsi,		8(%rcx)
	movq	%rdx,		16(%rcx)
	movq	%rcx,		24(%rcx)
	movq	%r8,		32(%rcx)
	movq	%r9,		40(%rcx)
	movq	%rbx,		48(%rcx)
	movq	%rbp,		56(%rcx)
	movq	%r10,		64(%rcx)
	movq	%r11,		72(%rcx)
	movq	%r12,		80(%rcx)
	movq	%r13,		88(%rcx)
	movq	%r14,		96(%rcx)
	movq	%r15,		104(%rcx)

	movq	0(%rsp),	%r15
	movq	%r15,		112(%rcx)

	leaq	8(%rsp),	%r15
	movq	%r15,		120(%rcx)

	movq	104(%rcx),	%r15
	ret


.globl platform_setcontext
platform_setcontext:
	movq	0(%rcx),	%rdi
	movq	8(%rcx),	%rsi
	movq	16(%rcx),	%rdx
	movq	32(%rcx),	%r8
	movq	40(%rcx),	%r9
	movq	48(%rcx),	%rbx
	movq	56(%rcx),	%rbp
	movq	64(%rcx),	%r10
	movq	72(%rcx),	%r11
	movq	80(%rcx),	%r12
	movq	88(%rcx),	%r13
	movq	96(%rcx),	%r14
	movq	104(%rcx),	%r15

	movq	120(%rcx),	%rsp
	pushq	112(%rcx)

	movq	24(%rcx),	%rcx
	ret
