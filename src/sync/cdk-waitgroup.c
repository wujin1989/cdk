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

cdk_waitgroup_t* cdk_waitgroup_create(void) {
    cdk_waitgroup_t* wg = malloc(sizeof(cdk_waitgroup_t));
    if (!wg) {
        return NULL;
    }
    wg->cnt = 0;
    mtx_init(&wg->mtx, mtx_plain);
    cnd_init(&wg->cnd);

    return wg;
}

void cdk_waitgroup_destroy(cdk_waitgroup_t* wg) {
    if (!wg) {
        return;
    }
    mtx_destroy(&wg->mtx);
    cnd_destroy(&wg->cnd);
    wg->cnt = 0;

    free(wg);
}

void cdk_waitgroup_add(cdk_waitgroup_t* wg, int delta) {
    mtx_lock(&wg->mtx);
    wg->cnt += delta;
    mtx_unlock(&wg->mtx);
}

void cdk_waitgroup_done(cdk_waitgroup_t* wg) {
    mtx_lock(&wg->mtx);
    wg->cnt--;
    if (wg->cnt == 0) {
        cnd_broadcast(&wg->cnd);
    }
    mtx_unlock(&wg->mtx);
}

void cdk_waitgroup_wait(cdk_waitgroup_t* wg) {
    mtx_lock(&wg->mtx);
    while (wg->cnt) {
        cnd_wait(&wg->cnd, &wg->mtx);
    }
    mtx_unlock(&wg->mtx);
}
