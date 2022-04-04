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

#ifndef __CDK_SYNC_H__
#define __CDK_SYNC_H__

#include "cdk-types.h"

/**
 *  create a mutex.
 *
 *  @param m [in/out].
 *  @return N/A.
 */
extern void cdk_mtx_init(mtx_t* restrict m);

/**
 *  destroy a mutex.
 *
 *  @param m [in].
 *  @return N/A.
 */
extern void cdk_mtx_destroy(mtx_t* m);

/**
 *  lock a mutex.
 *
 *  @param m [in].
 *  @return N/A.
 */
extern void cdk_mtx_lock(mtx_t* m);

/**
 *  unlock a mutex.
 *
 *  @param m [in].
 *  @return N/A.
 */
extern void cdk_mtx_unlock(mtx_t* m);

/**
 *  create a read-write lock.
 *
 *  @param rw [in/out].
 *  @return N/A.
 */
extern void cdk_rwlock_init(rwlock_t* restrict rw);

/**
 *  destroy a read-write lock.
 *
 *  @param rw [in].
 *  @return N/A.
 */
extern void cdk_rwlock_destroy(rwlock_t* rw);

/**
 *  lock a read-write lock object for reading.
 *
 *  @param rw [in].
 *  @return N/A.
 */
extern void cdk_rwlock_rlock(rwlock_t* rw);

/**
 *  unlock a read-write lock object for reading.
 *
 *  @param rw [in].
 *  @return N/A.
 */
extern void cdk_rwlock_runlock(rwlock_t* rw);

/**
 *  lock a read-write lock object for writing.
 *
 *  @param rw [in].
 *  @return N/A.
 */
extern void cdk_rwlock_wlock(rwlock_t* rw);

/**
 *  unlock a read-write lock object for writing.
 *
 *  @param rw [in].
 *  @return N/A.
 */
extern void cdk_rwlock_wunlock(rwlock_t* rw);

/**
 *  create a condition variable.
 *
 *  @param c [in/out].
 *  @return N/A.
 */
extern void cdk_cnd_init(cnd_t* restrict c);

/**
 *  destroy a condition variable.
 *
 *  @param c [in].
 *  @return N/A.
 */
extern void cdk_cnd_destroy(cnd_t* c);

/**
 *  unblocks one of the threads that are blocked on the condition variable.
 *
 *  @param c [in].
 *  @return N/A.
 */
extern void cdk_cnd_signal(cnd_t* c);

/**
 *  unblocks all of the threads that are blocked on the condition variable.
 *
 *  @param c [in].
 *  @return N/A.
 */
extern void cdk_cnd_broadcast(cnd_t* c);

/**
 *  the function atomically unlocks the mutex pointed to by m 
 *  and blocks until the condition variable pointed to by c is signaled by a call to cdk_cnd_signal or to cdk_cnd_broadcast.
 *  when the calling thread becomes unblocked it locks the  mutex pointed to by m before it returns. 
 *
 *  @param c [in].
 *  @param m [in].
 *  @return N/A.
 */
extern void cdk_cnd_wait(cnd_t* restrict c, mtx_t* restrict m);

#endif /* __CDK_SYNC_H__ */
