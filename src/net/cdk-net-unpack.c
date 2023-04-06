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
#include "cdk/cdk-utils.h"
#include "cdk/encoding/cdk-varint.h"

static void __net_unpack_fixedlen(cdk_net_conn_t* conn) {

	char* head = conn->tcp.rxbuf.buf;
	char* tail = (char*)conn->tcp.rxbuf.buf + conn->tcp.rxbuf.off;
	char* tmp = head;

	uint32_t accumulated = (uint32_t)(tail - head);

	while (true) {

		if (accumulated < conn->tcp.unpacker.fixedlen.len) {
			break;
		}
		conn->h->on_read(conn, tmp, conn->tcp.unpacker.fixedlen.len);

		tmp += conn->tcp.unpacker.fixedlen.len;
		accumulated -= conn->tcp.unpacker.fixedlen.len;
	}
	if (tmp == head) {
		return;
	}
	conn->tcp.rxbuf.off = accumulated;
	if (accumulated) {
		memmove(conn->tcp.rxbuf.buf, tmp, accumulated);
	}
	return;
}

static void __net_unpack_delimiter(cdk_net_conn_t* conn) {

	char* head = conn->tcp.rxbuf.buf;
	char* tail = (char*)conn->tcp.rxbuf.buf + conn->tcp.rxbuf.off;
	char* tmp = head;

	size_t dlen = strlen(conn->tcp.unpacker.delimiter.delimiter);

	uint32_t accumulated = (uint32_t)(tail - head);
	if (accumulated < dlen) {
		return;
	}
	/**
	 * for performance, thus split buffer by KMP.
	 */
	int* next = malloc(dlen * sizeof(int));
	memset(next, 0, dlen * sizeof(int));
	int j = 0;

	for (int i = 1; i < dlen; i++) {

		while (j > 0 && conn->tcp.unpacker.delimiter.delimiter[i] != conn->tcp.unpacker.delimiter.delimiter[j]) {
			j = next[j - 1];
		}
		if (conn->tcp.unpacker.delimiter.delimiter[i] == conn->tcp.unpacker.delimiter.delimiter[j]) {
			j++;
		}
		next[i] = j;
	}
	j = 0;
	for (uint32_t i = 0; i < accumulated; i++) {

		while (j > 0 && tmp[i] != conn->tcp.unpacker.delimiter.delimiter[j]) {
			j = next[j - 1];
		}
		if (tmp[i] == conn->tcp.unpacker.delimiter.delimiter[j]) {
			j++;
		}
		if (j == dlen) {

			conn->h->on_read(conn, tmp, ((i - dlen + 1) + dlen));

			tmp += (i - dlen + 1) + dlen;
			accumulated -= (uint32_t)((i - dlen + 1) + dlen);

			i = -1;
			j = 0;
		}
	}
	free(next);
	next = NULL;
	if (tmp == head) {
		return;
	}
	conn->tcp.rxbuf.off = accumulated;
	if (accumulated) {
		memmove(conn->tcp.rxbuf.buf, tmp, accumulated);
	}
	return;
}

static void __net_unpack_lengthfield(cdk_net_conn_t* conn) {

	uint32_t fs; /* frame size   */
	uint32_t hs; /* header size  */
	uint32_t ps; /* payload size */

	char* head = conn->tcp.rxbuf.buf;
	char* tail = (char*)conn->tcp.rxbuf.buf + conn->tcp.rxbuf.off;
	char* tmp = head;

	uint32_t accumulated = (uint32_t)(tail - head);

	while (true) {
		if (accumulated < conn->tcp.unpacker.lengthfield.payload) {
			break;
		}
		hs = conn->tcp.unpacker.lengthfield.payload;
		ps = 0;

		if (conn->tcp.unpacker.lengthfield.coding == LEN_FIELD_FIXEDINT) {

			ps = *((uint32_t*)(tmp + conn->tcp.unpacker.lengthfield.offset));
			//0 means little-endian, 1 means big-endian.
			if (!cdk_utils_byteorder()) {
				ps = ntohl(ps);
			}
		}
		if (conn->tcp.unpacker.lengthfield.coding == LEN_FIELD_VARINT) {

			int flexible = (int)(tail - (tmp + conn->tcp.unpacker.lengthfield.offset));

			ps = (uint32_t)cdk_varint_decode(tmp + conn->tcp.unpacker.lengthfield.offset, &flexible);

			if (!cdk_utils_byteorder()) {
				ps = ntohl(ps);
			}
			hs = conn->tcp.unpacker.lengthfield.payload + flexible - conn->tcp.unpacker.lengthfield.size;
		}
		fs = hs + ps + conn->tcp.unpacker.lengthfield.adj;

		if (fs > conn->tcp.rxbuf.len) {
			abort();
		}
		if (accumulated < fs) {
			break;
		}
		conn->h->on_read(conn, tmp, fs);
		tmp += fs;
		accumulated -= fs;
	}
	if (tmp == head) {
		return;
	}
	conn->tcp.rxbuf.off = accumulated;
	if (accumulated) {
		memmove(conn->tcp.rxbuf.buf, tmp, accumulated);
	}
	return;
}

static void __net_unpack_userdefined(cdk_net_conn_t* conn) {

	conn->tcp.unpacker.userdefined.unpack(conn);
}

void cdk_net_unpack(cdk_net_conn_t* conn) {

	switch (conn->tcp.unpacker.type)
	{
	case UNPACK_TYPE_FIXEDLEN: {
		__net_unpack_fixedlen(conn);
		break;
	}
	case UNPACK_TYPE_DELIMITER: {
		__net_unpack_delimiter(conn);
		break;
	}
	case UNPACK_TYPE_LENGTHFIELD: {
		__net_unpack_lengthfield(conn);
		break;
	}
	case UNPACK_TYPE_USERDEFINED: {
		__net_unpack_userdefined(conn);
		break;
	}
	default:
		abort();
	}
	return;
}