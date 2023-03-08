#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cdk.h"

int main() {
    const char* message = "hello world";
    size_t message_len = strlen(message);

    cdk_sha256_ctx_t ctx;
    cdk_sha256_init(&ctx);
    cdk_sha256_update(&ctx, (const uint8_t*)message, message_len);

    uint8_t hash[CDK_SHA256_BLOCK_SIZE];
    cdk_sha256_final(&ctx, hash);

    printf("message: %s\n", message);
    printf("hash: ");
    for (int i = 0; i < CDK_SHA256_BLOCK_SIZE; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");

    return 0;
}
