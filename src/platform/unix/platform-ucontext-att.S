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
	movq	%rdi,		0(%rdi)
	movq	%rsi,		8(%rdi)
	movq	%rdx,		16(%rdi)
	movq	%rcx,		24(%rdi)
	movq	%r8,		32(%rdi)
	movq	%r9,		40(%rdi)
		
	movq	%rbx,		48(%rdi)
	movq	%rbp,		56(%rdi)
	movq	%r10,		64(%rdi)
	movq	%r11,		72(%rdi)
	movq	%r12,		80(%rdi)
	movq	%r13,		88(%rdi)
	movq	%r14,		96(%rdi)
	movq	%r15,		104(%rdi)
		
	movq	(%rsp),		%rcx
	movq	%rcx,		112(%rdi)

	leaq	8(%rsp),	%rcx
	movq	%rcx,		120(%rdi)

	movq	24(%rdi),	%rcx
	ret


.globl platform_setcontext
platform_setcontext:
	movq	8(%rdi),	%rsi
	movq	16(%rdi),	%rdx
	movq	24(%rdi),	%rcx
	movq	32(%rdi),	%r8
	movq	40(%rdi),	%r9
		
	movq	48(%rdi),	%rbx
	movq	56(%rdi),	%rbp
	movq	64(%rdi),	%r10
	movq	72(%rdi),	%r11
	movq	80(%rdi),	%r12
	movq	88(%rdi),	%r13
	movq	96(%rdi),	%r14
	movq	104(%rdi),	%r15
		
	movq	120(%rdi),	%rsp
	pushq	112(%rdi)

	movq	0(%rdi),	%rdi
	ret
