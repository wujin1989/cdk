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
#include "cdk/cdk-time.h"
#include "cdk/container/cdk-heap.h"
#include "cdk/cdk-logger.h"

static int _min_heapcmp(cdk_heap_node_t* a, cdk_heap_node_t* b) {
	cdk_timer_t* ta = cdk_heap_data(a, cdk_timer_t, node);
	cdk_timer_t* tb = cdk_heap_data(b, cdk_timer_t, node);

	if ((ta->birth + ta->expire) < (tb->birth + tb->expire)) {
		return 1;
	}
	if ((ta->birth + ta->expire) == (tb->birth + tb->expire)) {
		if ((ta->id < tb->id)) {
			return 1;
		}
	}
	return 0;
}

cdk_timermgr_t* cdk_timer_manager_create(void) {
    cdk_timermgr_t* timermgr = malloc(sizeof(cdk_timermgr_t));
    if (timermgr) {
        cdk_heap_init(&timermgr->heap, _min_heapcmp);
        mtx_init(&timermgr->mtx, mtx_plain);
        timermgr->ntimers = 0;
	}
    return timermgr;
}

void cdk_timer_manager_destroy(cdk_timermgr_t* timermgr) {
    timermgr->ntimers = 0;
    mtx_destroy(&timermgr->mtx);

    if (timermgr) {
        free(timermgr);
        timermgr = NULL;
	}
}

cdk_timer_t* cdk_timer_add(
    cdk_timermgr_t* timermgr,
    void            (*routine)(void*),
    void*           param,
    size_t          expire,
    bool            repeat) {
	cdk_timer_t* timer = malloc(sizeof(cdk_timer_t));
	if (timer) {
        mtx_lock(&timermgr->mtx);
		timer->routine = routine;
		timer->param = param;
		timer->birth = cdk_time_now();
        timer->id = timermgr->ntimers++;
		timer->expire = expire;
		timer->repeat = repeat;

		cdk_heap_insert(&timermgr->heap, &timer->node);
        mtx_unlock(&timermgr->mtx);
	}
    return timer;
}

void cdk_timer_del(cdk_timermgr_t* timermgr, cdk_timer_t* timer) {
    mtx_lock(&timermgr->mtx);
    cdk_heap_remove(&timermgr->heap, &timer->node);
    timermgr->ntimers--;
    mtx_unlock(&timermgr->mtx);

	free(timer);
	timer = NULL;
}

void cdk_timer_reset(
    cdk_timermgr_t* timermgr, cdk_timer_t* timer, size_t expire) {
    mtx_lock(&timermgr->mtx);
    cdk_heap_remove(&timermgr->heap, &timer->node);
	timer->birth = cdk_time_now();
	timer->expire = expire;
    cdk_heap_insert(&timermgr->heap, &timer->node);
    mtx_unlock(&timermgr->mtx);
}

bool cdk_timer_empty(cdk_timermgr_t* timermgr) {
    mtx_lock(&timermgr->mtx);
    bool empty = cdk_heap_empty(&timermgr->heap);
    mtx_unlock(&timermgr->mtx);
    return empty;
}

cdk_timer_t* cdk_timer_min(cdk_timermgr_t* timermgr) {
    mtx_lock(&timermgr->mtx);
    cdk_timer_t* timer = cdk_heap_data(cdk_heap_min(&timermgr->heap), cdk_timer_t, node);
    mtx_unlock(&timermgr->mtx);
    return timer;
}