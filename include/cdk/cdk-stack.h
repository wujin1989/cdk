/** Copyright (c) 2022, Wu Jin <wujin.developer@gmail.com>
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

#ifndef __CDK_STACK_H__
#define __CDK_STACK_H__

#include "cdk/cdk-list.h"

#define cdk_stack_data     cdk_list_data

/**
 *  create a null stack.
 *
 *  @param s [in] current stack.
 *  @return N/A.
 */
extern void cdk_stack_create(filo_t* s);
/**
 *  initialize a stack node.
 *
 *  @param x [in].
 *  @return N/A.
 */
extern void cdk_stack_init_node(filo_node_t* x);
/**
 *  push a element into stack.
 *
 *  @param s [in] current stack.
 *  @param x [in] new node.
 *  @return N/A.
 */
extern void cdk_stack_push(filo_t* s, filo_node_t* x);
/**
 *  pop a element.
 *
 *  @param s [in] current stack.
 *  @return stack-top's node.
 */
extern filo_node_t* cdk_stack_pop(filo_t* s);
/**
 *  check if the stack is empty.
 *
 *  @param s [in] current stack.
 *  @return if stack is empty return true, otherwis return false.
 */
extern bool cdk_stack_empty(filo_t* s);

#endif /* __CDK_STACK_H__ */
