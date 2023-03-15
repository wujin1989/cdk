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

_Pragma("once")

#include "cdk/time/cdk-timer.h"
#include "cdk/cdk-process.h"
#include "cdk/cdk-math.h"
#include "cdk/time/cdk-time.h"
#include "cdk/cdk-logger.h"
#include "cdk/thread/cdk-threadpool.h"
#include "cdk/thread/cdk-systid.h"
#include "cdk/cdk-types.h"
#include "cdk/cdk-net.h"
#include "cdk/container/cdk-queue.h"
#include "cdk/container/cdk-stack.h"
#include "cdk/cdk-sysinfo.h"
#include "cdk/container/cdk-list.h"
#include "cdk/cdk-memory.h"
#include "cdk/cdk-loader.h"
#include "cdk/cdk-file.h"
#include "cdk/container/cdk-rbtree.h"
#include "cdk/cdk-string.h"
#include "cdk/encoding/cdk-varint.h"
#include "cdk/thread/cdk-rwlock.h"
#include "cdk/crypto/cdk-sha256.h"
#include "cdk/crypto/cdk-sha1.h"
#include "cdk/encoding/cdk-base64.h"
#include "cdk/encoding/cdk-json.h"

/**
 * When Apple and Microsoft support C11 threads, it will be removed from CDK. 
 */
#include "cdk/deprecated/threads.h"