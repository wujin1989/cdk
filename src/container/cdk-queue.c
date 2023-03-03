/** Copyright (c) 2023-2033, Wu Jin <wujin.developer@gmail.com>
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

#include "cdk/container/cdk-list.h"

void cdk_queue_create(cdk_queue_t* q) {

	cdk_list_create(q);
}

void cdk_queue_enqueue(cdk_queue_t* q, cdk_queue_node_t* x) {

	cdk_list_insert_tail(q, x);
}

bool cdk_queue_empty(cdk_queue_t* q) {

	return cdk_list_empty(q);
}

cdk_queue_node_t* cdk_queue_dequeue(cdk_queue_t* q) {

	if (cdk_queue_empty(q)) {
		return NULL;
	}
	cdk_queue_node_t* n = cdk_list_head(q);

	cdk_list_remove(n);
	return n;
}

