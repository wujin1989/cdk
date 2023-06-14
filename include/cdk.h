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

#include <assert.h>
#include <complex.h>
#include <ctype.h>
#include <errno.h>
#include <fenv.h>
#include <float.h>
#include <inttypes.h>
#include <iso646.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <stdalign.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <tgmath.h>
//#include <threads.h>
/**
 * This is a backup, it will be removed when threads.h for vs and xcode are released.
 */
#include "cdk/deprecated/c11-threads.h"
#include <time.h>
/**
 * MacOSX not support.
 */
//#include <uchar.h>
#include <wchar.h>
#include <wctype.h>

#include "cdk/cdk-timer.h"
#include "cdk/cdk-process.h"
#include "cdk/cdk-time.h"
#include "cdk/cdk-logger.h"
#include "cdk/cdk-threadpool.h"
#include "cdk/cdk-types.h"
#include "cdk/cdk-utils.h"
#include "cdk/cdk-loader.h"
#include "cdk/sync/cdk-rwlock.h"
#include "cdk/sync/cdk-spinlock.h"
#include "cdk/container/cdk-queue.h"
#include "cdk/container/cdk-stack.h"
#include "cdk/container/cdk-list.h"
#include "cdk/container/cdk-rbtree.h"
#include "cdk/container/cdk-ringbuffer.h"
#include "cdk/encoding/cdk-varint.h"
#include "cdk/encoding/cdk-base64.h"
#include "cdk/crypto/cdk-sha256.h"
#include "cdk/crypto/cdk-sha1.h"
#include "cdk/net/cdk-net.h"
