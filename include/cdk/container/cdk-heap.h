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
#include <stddef.h>

#define cdk_heap_data(x, t, m)                              \
    ((t *) ((char *) (x) - offsetof(t, m)))

extern void cdk_heap_init(cdk_heap_t* heap, int (*cmp)(cdk_heap_node_t* a, cdk_heap_node_t* b));
extern void cdk_heap_insert(cdk_heap_t* heap, cdk_heap_node_t* node);
extern void cdk_heap_remove(cdk_heap_t* heap, cdk_heap_node_t* node);
extern void cdk_heap_dequeue(cdk_heap_t* heap);
extern bool cdk_heap_empty(cdk_heap_t* heap);
extern cdk_heap_node_t* cdk_heap_min(cdk_heap_t* heap);
