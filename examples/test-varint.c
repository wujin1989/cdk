#include "cdk.h"
#include <inttypes.h>

int main(void) {

	int pos = 0;
	int len;
	uint64_t number = 23113293191329;
	char* str = calloc(7, sizeof(char));

	len = cdk_varint_encode(number, str);

	uint64_t val = cdk_varint_decode(str, &pos);
	printf("len=%d, pos=%d, val= %"PRIu64"\n", len, pos, val);
	return 0;
}