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

#include "cdk/cdk-types.h"

void cdk_list_init(cdk_list_t* list) {
	(list)->next = (list);
	(list)->prev = (list);
}

void cdk_list_insert_head(cdk_list_t* list, cdk_list_node_t* node) {
	/* init node */
	(node)->next = (node);
	(node)->prev = (node);

	/* insert node */
	(node)->next = (list)->next;
	(node)->next->prev = (node);
	(node)->prev = (list);
	(list)->next = (node);
}

void cdk_list_insert_tail(cdk_list_t* list, cdk_list_node_t* node) {
	(node)->next = (node);
	(node)->prev = (node);

	(node)->prev = (list)->prev;
	(node)->prev->next = (node);
	(node)->next = (list);
	(list)->prev = (node);
}

bool cdk_list_empty(cdk_list_t* list) {
	return (list) == (list)->prev;
}

cdk_list_node_t* cdk_list_head(cdk_list_t* list) {
	return (list)->next;
}

cdk_list_node_t* cdk_list_tail(cdk_list_t* list) {
	return (list)->prev;
}

cdk_list_node_t* cdk_list_sentinel(cdk_list_t* list) {
	return (list);
}

void cdk_list_remove(cdk_list_node_t* node) {
	(node)->next->prev = (node)->prev;
	(node)->prev->next = (node)->next;
}

cdk_list_node_t* cdk_list_next(cdk_list_node_t* node) {
	return (node)->next;
}

cdk_list_node_t* cdk_list_prev(cdk_list_node_t* node) {
	return (node)->prev;
}
