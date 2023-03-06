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
#include "cdk/cdk-memory.h"
#include "cdk/cdk-rbtree.h"
#include "cdk/cdk-mtx.h"
#include "cdk/cdk-threadpool.h"

typedef struct cdk_timer_entry_s {

	void (*routine)(void*);
	void* arg;
	bool  repeat;

	struct cdk_rbtree_node_s node;
}cdk_timer_entry_t;

void cdk_timer_create(cdk_timer_t* timer, uint32_t workers) {

	cdk_rbtree_create(&timer->tree, RB_KEYTYPE_UINT64);
	cdk_thrdpool_create(&timer->pool, workers);
}

void cdk_timer_destroy(cdk_timer_t* timer) {

	cdk_thrdpool_destroy(&timer->pool);
}

void cdk_timer_add(cdk_timer_t* timer, void (*routine)(void*), void* arg, uint64_t expire, bool repeat) {

	cdk_timer_entry_t* entry;

	entry = cdk_memory_malloc(sizeof(cdk_timer_entry_t));
	
	entry->routine = routine;
	entry->arg     = arg;
	entry->repeat  = repeat;
	entry->node.rb_key.u64 = expire;

	cdk_rbtree_insert(&timer->tree, &entry->node);
}
