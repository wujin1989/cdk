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

#include "cdk/cdk-list.h"

void cdk_list_create(list_t* l) {

	(l)->n = (l);
	(l)->p = (l);
}

void cdk_list_insert_head(list_t* l, list_node_t* x) {

	/**
	 * init node;
	 */
	(x)->n     = (x);
	(x)->p     = (x);
	/**
	 * insert node;
	 */
	(x)->n     = (l)->n;
	(x)->p->n  = (x);
	(x)->p     = (l);
	(l)->n     = (x);
}

void cdk_list_insert_tail(list_t* l, list_node_t* x) {

	/**
	 * init node;
	 */
	(x)->n = (x);
	(x)->p = (x);
	/**
	 * insert node;
	 */
	(x)->p     = (l)->p;
	(x)->p->n  = (x);
	(x)->n     = (l);
	(l)->p     = (x);
}

bool cdk_list_empty(list_t* l) {

	return (l) == (l)->p;
}

list_node_t* cdk_list_head(list_t* l) {

	return (l)->n;
}

list_node_t* cdk_list_tail(list_t* l) {

	return (l)->p;
}

list_node_t* cdk_list_sentinel(list_t* l) {

	return (l);
}

void cdk_list_remove(list_node_t* x) {

	(x)->n->p   = (x)->p;
	(x)->p->n   = (x)->n;
}

list_node_t* cdk_list_next(list_node_t* x) {

	return (x)->n;
}

list_node_t* cdk_list_prev(list_node_t* x) {

	return (x)->p;
}