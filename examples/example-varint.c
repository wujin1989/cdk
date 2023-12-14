#include "cdk.h"

int main(void) {
	int pos = 0;
	int len = 0;
	uint64_t val = 0;
	uint64_t number = 23113293191329;
	char* str = calloc(64, sizeof(char));

	len = cdk_varint_encode(number, str);
	val = cdk_varint_decode(str, &pos);

	printf("len=%d, pos=%d, val= %"PRIu64"\n", len, pos, val);
	return 0;
}