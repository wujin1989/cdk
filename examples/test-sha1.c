#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cdk.h"

int main(void) {
    cdk_sha1_ctx_t ctx;
    cdk_sha1_init(&ctx);

    uint8_t data[] = "hello world";
    uint32_t len = (uint32_t)strlen(data);
    cdk_sha1_update(&ctx, data, len);

    unsigned char digest[20];
    cdk_sha1_final(&ctx, digest);

    printf("SHA1 digest: ");
    for (int i = 0; i < 20; i++) {
        printf("%02x", digest[i]);
    }
    printf("\n");

    return 0;
}
