#include <cdk.h>
#include <stdio.h>
#include <string.h>

static void encode() {

    char src[] = "CDK is a clean and concise cross-platform C development kits that complements the standard C library.";
    char dst[512] = { 0 };

    size_t dstlen;
    size_t srclen = strlen(src);
    
    cdk_base64_encode(src, srclen, dst, &dstlen);
    printf("dst: %s, dstlen: %zu\n", dst, dstlen);
}

static void decode() {

    char* src = "Q0RLIGlzIGEgY2xlYW4gYW5kIGNvbmNpc2UgY3Jvc3MtcGxhdGZvcm0gQyBkZXZlbG9wbWVudCBraXRzIHRoYXQgY29tcGxlbWVudHMgdGhlIHN0YW5kYXJkIEMgbGlicmFyeS4=";
    char dst[512] = { 0 };

    size_t dstlen;
    cdk_base64_decode(src, strlen(src), dst, &dstlen);

    printf("dst: %.*s\n", (int)dstlen, dst);
    return;
}

int main(void) {

    encode();
    decode();
	return 0;
}