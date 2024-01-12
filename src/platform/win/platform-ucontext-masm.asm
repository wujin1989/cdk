; Copyright (c), Wu Jin <wujin.developer@gmail.com>
;
;  Permission is hereby granted, free of charge, to any person obtaining a copy
;  of this software and associated documentation files (the "Software"), to
;  deal in the Software without restriction, including without limitation the
;  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
;  sell copies of the Software, and to permit persons to whom the Software is
;  furnished to do so, subject to the following conditions:
;
;  The above copyright notice and this permission notice shall be included in
;  all copies or substantial portions of the Software.
;
;  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
;  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
;  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
;  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
;  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
;  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
;  IN THE SOFTWARE.
;

 .code

platform_getcontext proc
	mov	[rcx],		    rdi
	mov	[rcx + 8],		rsi
	mov [rcx + 16],		rdx
	mov [rcx + 24],		rcx
	mov [rcx + 32],		r8
	mov [rcx + 40],		r9
	mov	[rcx + 48],		rbx
	mov	[rcx + 56],		rbp
	mov	[rcx + 64],		r10
	mov	[rcx + 72],		r11
	mov	[rcx + 80],		r12
	mov	[rcx + 88],		r13
	mov	[rcx + 96],		r14
	mov	[rcx + 104],	r15
	
	mov	r15,			[rsp]
	mov	[rcx + 112],	r15

	lea	r15,			[rsp + 8]
	mov	[rcx + 120],	r15

	mov	r15,			[rcx + 104]
	ret
platform_getcontext endp

platform_setcontext	proc
	mov	rdi,		[rcx]
	mov	rsi,		[rcx + 8]
	mov	rdx,		[rcx + 16]
	
	mov	r8,			[rcx + 32]
	mov	r9,			[rcx + 40]
	mov	rbx,		[rcx + 48]
	mov	rbp,		[rcx + 56]
	mov	r10,		[rcx + 64]
	mov	r11,		[rcx + 72]
	mov	r12,		[rcx + 80]
	mov	r13,		[rcx + 88]
	mov	r14,		[rcx + 96]
	mov	r15,		[rcx + 104]

	mov	rsp,		[rcx + 120]
	push			[rcx + 112]

	mov	rcx,		[rcx + 24]
	ret
platform_setcontext endp	

end