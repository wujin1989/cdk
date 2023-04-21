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

#include "cdk/container/cdk-list.h"

#define cdk_rbtree_data   cdk_list_data

extern int default_keycmp_i8(cdk_rbtree_node_key_t* k1, cdk_rbtree_node_key_t* k2);
extern int default_keycmp_i16(cdk_rbtree_node_key_t* k1, cdk_rbtree_node_key_t* k2);
extern int default_keycmp_i32(cdk_rbtree_node_key_t* k1, cdk_rbtree_node_key_t* k2);
extern int default_keycmp_i64(cdk_rbtree_node_key_t* k1, cdk_rbtree_node_key_t* k2);
extern int default_keycmp_u8(cdk_rbtree_node_key_t* k1, cdk_rbtree_node_key_t* k2);
extern int default_keycmp_u16(cdk_rbtree_node_key_t* k1, cdk_rbtree_node_key_t* k2);
extern int default_keycmp_u32(cdk_rbtree_node_key_t* k1, cdk_rbtree_node_key_t* k2);
extern int default_keycmp_u64(cdk_rbtree_node_key_t* k1, cdk_rbtree_node_key_t* k2);
extern int default_keycmp_str(cdk_rbtree_node_key_t* k1, cdk_rbtree_node_key_t* k2);

extern void               cdk_rbtree_init(cdk_rbtree_t* tree, int(*keycmp)(cdk_rbtree_node_key_t*, cdk_rbtree_node_key_t*));
extern void               cdk_rbtree_insert(cdk_rbtree_t* tree, cdk_rbtree_node_t* node);
extern cdk_rbtree_node_t* cdk_rbtree_find(cdk_rbtree_t* tree, cdk_rbtree_node_key_t key);
extern void               cdk_rbtree_erase(cdk_rbtree_t* tree, cdk_rbtree_node_t* node);
extern cdk_rbtree_node_t* cdk_rbtree_next(cdk_rbtree_node_t* node);
extern cdk_rbtree_node_t* cdk_rbtree_prev(cdk_rbtree_node_t* node);
extern cdk_rbtree_node_t* cdk_rbtree_first(cdk_rbtree_t* tree);
extern cdk_rbtree_node_t* cdk_rbtree_last(cdk_rbtree_t* tree);
extern bool               cdk_rbtree_empty(cdk_rbtree_t* tree);

