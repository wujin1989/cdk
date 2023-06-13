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

extern void     cdk_ringbuf_create(cdk_ringbuf_t* ring, uint32_t esize, uint32_t bufsize);
extern void     cdk_ringbuf_destroy(cdk_ringbuf_t* ring);
extern uint32_t cdk_ringbuf_len(cdk_ringbuf_t* ring);
extern uint32_t cdk_ringbuf_cap(cdk_ringbuf_t* ring);
extern uint32_t cdk_ringbuf_avail(cdk_ringbuf_t* ring);
extern bool     cdk_ringbuf_full(cdk_ringbuf_t* ring);
extern bool     cdk_ringbuf_empty(cdk_ringbuf_t* ring);
extern uint32_t cdk_ringbuf_write(cdk_ringbuf_t* ring, const void* buf, uint32_t entry_count);
extern uint32_t cdk_ringbuf_read(cdk_ringbuf_t* ring, void* buf, uint32_t entry_count);
