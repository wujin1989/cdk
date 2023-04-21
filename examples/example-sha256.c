#include "cdk.h"

int main() {
    const char* message = "hello world";
    size_t message_len = strlen(message);

    cdk_sha256_t ctx;
    cdk_sha256_init(&ctx);
    cdk_sha256_update(&ctx, (const uint8_t*)message, message_len);

    uint8_t hash[32];
    cdk_sha256_final(&ctx, hash);

    printf("message: %s\n", message);
    printf("hash: ");
    for (int i = 0; i < 32; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");

    return 0;
}