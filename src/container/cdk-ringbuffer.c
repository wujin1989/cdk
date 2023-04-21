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

#include <string.h>
#include <stdint.h>
#include "cdk/cdk-types.h"

static uint32_t __ringbuf_rounddown_pow_of_two(uint32_t n) {

    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    return (n + 1) >> 1;
}

static void __ringbuf_internal_write(cdk_ringbuf_t* ring, const void* src, uint32_t len, uint32_t off) {

    uint32_t size  = ring->m + 1;
    uint32_t esize = ring->esz;
    uint32_t l;

    off &= ring->m;
    if (esize != 1) {
        off  *= esize;
        size *= esize;
        len  *= esize;
    }
    l = (len) < (size - off) ? (len) : (size - off);

    memcpy(ring->b + off, src, l);
    memcpy(ring->b, (uint8_t*)src + l, len - l);
}

static void __ringbuf_internal_read(cdk_ringbuf_t* ring, void* dst, uint32_t len, uint32_t off) {

    uint32_t size  = ring->m + 1;
    uint32_t esize = ring->m;
    uint32_t l;

    off &= ring->m;
    if (esize != 1) {
        off  *= esize;
        size *= esize;
        len  *= esize;
    }
    l = (len) < (size - off) ? (len) : (size - off);

    memcpy(dst, ring->b + off, l);
    memcpy((uint8_t*)dst + l, ring->b, len - l);
}

static uint32_t __ringbuf_internal_read_peek(cdk_ringbuf_t* ring, void* buf, uint32_t len) {

    uint32_t l;
    l = ring->w - ring->r;
    if (len > l) {
        len = l;
    }
    cdk_ringbuf_internal_read(ring, buf, len, ring->r);
    return len;
}

void cdk_ringbuf_init(cdk_ringbuf_t* ring, uint32_t esize, void* buf, uint32_t bsize) {

    ring->b    = buf;
    ring->esz  = esize;
    ring->m    = __ringbuf_rounddown_pow_of_two(bsize / esize) - 1;
    ring->w    = 0;
    ring->r    = 0;
}

uint32_t cdk_ringbuf_len(cdk_ringbuf_t* ring) {

    return ring->w - ring->r;
}

uint32_t cdk_ringbuf_cap(cdk_ringbuf_t* ring) {

    return ring->m + 1;
}

uint32_t cdk_ringbuf_avail(cdk_ringbuf_t* ring) {

    return cdk_ringbuf_cap(ring) - cdk_ringbuf_len(ring);
}

bool cdk_ringbuf_full(cdk_ringbuf_t* ring) {

    return cdk_ringbuf_len(ring) > ring->m;
}

bool cdk_ringbuf_empty(cdk_ringbuf_t* ring) {

    return ring->w == ring->r;
}

uint32_t cdk_ringbuf_write(cdk_ringbuf_t* ring, const void* buf, uint32_t entry_count) {

    uint32_t avail = cdk_ringbuf_avail(ring);
    if (entry_count > avail) {
        entry_count = avail;
    }
    __ringbuf_internal_write(ring, buf, entry_count, ring->w);

    ring->w += entry_count;
    return entry_count;
}

uint32_t cdk_ringbuf_read(cdk_ringbuf_t* ring, void* buf, uint32_t entry_count) {

    entry_count = __ringbuf_internal_read_peek(ring, buf, entry_count);
    ring->r += entry_count;

    return entry_count;
}