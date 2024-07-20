#include "cdk.h"

#define BUFFERSIZE 64

typedef struct people_s {
    char name[8];
    int age;
} people_t;

int main(void) {
    cdk_ringbuf_t ring;
    cdk_ringbuf_create(&ring, sizeof(people_t), BUFFERSIZE);

    uint32_t len = cdk_ringbuf_len(&ring);
    printf("ringbuffer len: %u\n", len);

    uint32_t cap = cdk_ringbuf_cap(&ring);
    printf("ringbuffer cap: %u\n", cap);

    uint32_t avail = cdk_ringbuf_avail(&ring);
    printf("ringbuffer avail: %u\n", avail);

    bool isfull = cdk_ringbuf_full(&ring);
    printf("ringbuffer is full? %s\n", isfull ? "true" : "false");

    bool isempty = cdk_ringbuf_empty(&ring);
    printf("ringbuffer is empty? %s\n", isempty ? "true" : "false");

    people_t p1 = { .name = "John", .age = 25 };
    people_t p2 = { .name = "Alice", .age = 30 };
    people_t p3 = { .name = "Bob", .age = 35 };
    people_t p4 = { .name = "Han", .age = 35 };
    people_t p5 = { .name = "Jun", .age = 35 };

    cdk_ringbuf_write(&ring, &p1, 1);
    cdk_ringbuf_write(&ring, &p2, 1);
    cdk_ringbuf_write(&ring, &p3, 1);
    cdk_ringbuf_write(&ring, &p4, 1);

    if (cdk_ringbuf_write(&ring, &p5, 1) == 0) {
        printf("write failed, because ringbuf is full\n");
    }
    printf("ringbuffer len: %u\n", cdk_ringbuf_len(&ring));

    people_t peoples[2];
    cdk_ringbuf_read(&ring, peoples, 2);

    for (int i = 0; i < 2; i++) {
        printf("name: %s, age: %d\n", peoples[i].name, peoples[i].age);
    }
    printf("ringbuffer avail: %u\n", cdk_ringbuf_avail(&ring));
    cdk_ringbuf_destroy(&ring);
    return 0;
}
