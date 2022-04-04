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

#ifndef __CDK_QUEUE_H__
#define __CDK_QUEUE_H__

#include "cdk/cdk-list.h"

#define cdk_queue_data     cdk_list_data

/**
 *  create a null queue.
 *
 *  @param q [in] current queue.
 *  @return N/A.
 */
extern void cdk_queue_create(fifo_t* q);
/**
 *  initialize a queue node.
 *
 *  @param x [in].
 *  @return N/A.
 */
extern void cdk_queue_init_node(fifo_node_t* x);
/**
 *  enqueue a element.
 *
 *  @param q [in] current queue.
 *  @param x [in] new node.
 *  @return N/A.
 */
extern void cdk_queue_enqueue(fifo_t* q, fifo_node_t* x);
/**
 *  dequeue a element.
 *
 *  @param q [in] current queue.
 *  @return queue's first node.
 */
extern fifo_node_t* cdk_queue_dequeue(fifo_t* q);
/**
 *  check if the queue is empty.
 *
 *  @param q [in] current queue.
 *  @return if queue is empty return true, otherwis return false.
 */
extern bool cdk_queue_empty(fifo_t* q);

#endif /* __CDK_QUEUE_H__ */
