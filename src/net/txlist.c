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

#include "txlist.h"
#include "cdk/container/cdk-list.h"

void txlist_create(cdk_list_t* list) {
	cdk_list_init(list);
}

void txlist_destroy(cdk_list_t* list) {
	while (!txlist_empty(list)) {
		txlist_node_t* e = cdk_list_data(cdk_list_head(list), txlist_node_t, n);
		txlist_remove(e);
	}
}

void txlist_insert(cdk_list_t* list, void* data, size_t size, bool totail) {
	txlist_node_t* node = malloc(sizeof(txlist_node_t) + size);
	if (node) {
		memset(node, 0, sizeof(txlist_node_t) + size);
		memcpy(node->buf, data, size);
		node->len = size;
		if (totail) {
			cdk_list_insert_tail(list, &(node->n));
		} else {
			cdk_list_insert_head(list, &(node->n));
		}
	}
}

void txlist_remove(txlist_node_t* node) {
	cdk_list_remove(&(node->n));
	free(node);
	node = NULL;
}

bool txlist_empty(cdk_list_t* list) {
	return cdk_list_empty(list);
}