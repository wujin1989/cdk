#include "cdk.h"

int main() {
    char* message = "123456789";
    size_t message_len = strlen(message);

    cdk_sha256_t ctx;
    cdk_sha256_init(&ctx);
    cdk_sha256_update(&ctx, (uint8_t*)message, message_len);

    uint8_t digest[32];
    cdk_sha256_final(&ctx, digest);

    printf("message: %s\n", message);
    printf("digest: ");
    for (int i = 0; i < 32; i++) {
        printf("%02x", digest[i]);
    }
    printf("\n");

    return 0;
}
