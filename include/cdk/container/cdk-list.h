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

#define cdk_list_data(x, t, m)                              \
    ((t *) ((char *) (x) - offsetof(t, m)))

extern void             cdk_list_init(cdk_list_t* l);
extern void             cdk_list_insert_head(cdk_list_t* l, cdk_list_node_t* x);
extern void             cdk_list_insert_tail(cdk_list_t* l, cdk_list_node_t* x);
extern bool             cdk_list_empty(cdk_list_t* l);
extern cdk_list_node_t* cdk_list_head(cdk_list_t* l);
extern cdk_list_node_t* cdk_list_tail(cdk_list_t* l);
extern cdk_list_node_t* cdk_list_next(cdk_list_node_t* x);
extern cdk_list_node_t* cdk_list_prev(cdk_list_node_t* x);
extern void             cdk_list_remove(cdk_list_node_t* x);
extern cdk_list_node_t* cdk_list_sentinel(cdk_list_t* l);

