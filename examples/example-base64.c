#include <cdk.h>

static void _encode() {
    uint8_t src[] = "CDK is a clean and concise cross-platform C development "
                    "kits that complements the standard C library.";
    uint8_t dst[512] = {0};
    size_t  dstlen = 0;

    cdk_base64_encode(src, sizeof(src) - 1, dst, &dstlen);
    printf("dst: %s, dstlen: %zu\n", dst, dstlen);
}

static void _decode() {
    uint8_t src[] =
        "Q0RLIGlzIGEgY2xlYW4gYW5kIGNvbmNpc2UgY3Jvc3MtcGxhdGZvcm0gQyBkZXZlbG9wbW"
        "VudCBraXRzIHRoYXQgY29tcGxlbWVudHMgdGhlIHN0YW5kYXJkIEMgbGlicmFyeS4=";
    uint8_t dst[512] = {0};
    size_t  dstlen;

    cdk_base64_decode(src, sizeof(src) - 1, dst, &dstlen);
    printf("dst2: %s\n", dst);
}

int main(void) {
    _encode();
    _decode();
    return 0;
}