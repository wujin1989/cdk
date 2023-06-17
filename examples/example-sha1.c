#include "cdk.h"

int main(void) {
    cdk_sha1_t ctx;
    cdk_sha1_init(&ctx);

    char* data = "hello world";
    size_t len = strlen(data);
    cdk_sha1_update(&ctx, (uint8_t*)data, len);

    uint8_t digest[20];
    cdk_sha1_final(&ctx, digest);

    printf("SHA1 digest: ");
    for (int i = 0; i < 20; i++) {
        printf("%02x", digest[i]);
    }
    printf("\n");

    return 0;
}
