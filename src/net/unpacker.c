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

static inline void _fixedlen_unpack(cdk_channel_t* channel) {
	char* head = channel->rxbuf.buf;
	char* tail = (char*)channel->rxbuf.buf + channel->rxbuf.off;
	char* tmp = head;

	uint32_t accumulated = (uint32_t)(tail - head);
	while (true) {
		if (accumulated < channel->handler->unpacker->fixedlen.len) {
			break;
		}
		if (channel->handler->on_read) {
			channel->handler->on_read(channel, tmp, channel->handler->unpacker->fixedlen.len);
		}
		tmp += channel->handler->unpacker->fixedlen.len;
		accumulated -= channel->handler->unpacker->fixedlen.len;
	}
	if (tmp == head) {
		return;
	}
	channel->rxbuf.off = accumulated;
	if (accumulated) {
		memmove(channel->rxbuf.buf, tmp, accumulated);
	}
}

static inline void _delimiter_unpack(cdk_channel_t* channel) {
	char* head = channel->rxbuf.buf;
	char* tail = (char*)channel->rxbuf.buf + channel->rxbuf.off;
	char* tmp = head;

	size_t dlen = strlen(channel->handler->unpacker->delimiter.delimiter);

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

		while (j > 0 && channel->handler->unpacker->delimiter.delimiter[i] != channel->handler->unpacker->delimiter.delimiter[j]) {
			j = next[j - 1];
		}
		if (channel->handler->unpacker->delimiter.delimiter[i] == channel->handler->unpacker->delimiter.delimiter[j]) {
			j++;
		}
		next[i] = j;
	}
	j = 0;
	for (uint32_t i = 0; i < accumulated; i++) {
		while (j > 0 && tmp[i] != channel->handler->unpacker->delimiter.delimiter[j]) {
			j = next[j - 1];
		}
		if (tmp[i] == channel->handler->unpacker->delimiter.delimiter[j]) {
			j++;
		}
		if (j == dlen) {
			if (channel->handler->on_read) {
				channel->handler->on_read(channel, tmp, ((i - dlen + 1) + dlen));
			}
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
	channel->rxbuf.off = accumulated;
	if (accumulated) {
		memmove(channel->rxbuf.buf, tmp, accumulated);
	}
	return;
}

static inline void _lengthfield_unpack(cdk_channel_t* channel) {
	uint32_t fs; /* frame size   */
	uint32_t hs; /* header size  */
	uint32_t ps; /* payload size */

	char* head = channel->rxbuf.buf;
	char* tail = (char*)channel->rxbuf.buf + channel->rxbuf.off;
	char* tmp = head;

	uint32_t accumulated = (uint32_t)(tail - head);
	while (true) {
		if (accumulated < channel->handler->unpacker->lengthfield.payload) {
			break;
		}
		hs = channel->handler->unpacker->lengthfield.payload;
		ps = 0;
		if (channel->handler->unpacker->lengthfield.coding == MODE_FIXEDINT) {

			ps = *((uint32_t*)(tmp + channel->handler->unpacker->lengthfield.offset));
			//1 means little-endian, 0 means big-endian.
			if (cdk_utils_byteorder()) {
				ps = ntohl(ps);
			}
		}
		if (channel->handler->unpacker->lengthfield.coding == MODE_VARINT) {

			int flexible = (int)(tail - (tmp + channel->handler->unpacker->lengthfield.offset));

			ps = (uint32_t)cdk_varint_decode(tmp + channel->handler->unpacker->lengthfield.offset, &flexible);

			if (cdk_utils_byteorder()) {
				ps = ntohl(ps);
			}
			hs = channel->handler->unpacker->lengthfield.payload + flexible - channel->handler->unpacker->lengthfield.size;
		}
		fs = hs + ps + channel->handler->unpacker->lengthfield.adj;

		if (fs > channel->rxbuf.len) {
			abort();
		}
		if (accumulated < fs) {
			break;
		}
		if (channel->handler->on_read) {
			channel->handler->on_read(channel, tmp, fs);
		}
		tmp += fs;
		accumulated -= fs;
	}
	if (tmp == head) {
		return;
	}
	channel->rxbuf.off = accumulated;
	if (accumulated) {
		memmove(channel->rxbuf.buf, tmp, accumulated);
	}
	return;
}

static void _userdefined_unpack(cdk_channel_t* channel) {
	channel->handler->unpacker->userdefined.unpack(channel);
}

void unpacker_unpack(cdk_channel_t* channel) {
	switch (channel->handler->unpacker->type)
	{
    case UNPACKER_TYPE_FIXEDLEN: {
		_fixedlen_unpack(channel);
		break;
	}
    case UNPACKER_TYPE_DELIMITER: {
		_delimiter_unpack(channel);
		break;
	}
    case UNPACKER_TYPE_LENGTHFIELD: {
		_lengthfield_unpack(channel);
		break;
	}
    case UNPACKER_TYPE_USERDEFINED: {
		_userdefined_unpack(channel);
		break;
	}
	default:
		abort();
	}
	return;
}
