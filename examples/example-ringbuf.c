#include "cdk.h"

#define BUFFERSIZE 64

typedef struct people_s {
    char name[8];
    int age;
} people_t;

int main(void) {
    void* buf = malloc(BUFFERSIZE);

    cdk_ringbuf_t ring;
    cdk_ringbuf_init(&ring, sizeof(people_t), buf, BUFFERSIZE);

    printf("%zu\n", sizeof(people_t));

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

    people_t obj1 = { .name = "John", .age = 25 };
    people_t obj2 = { .name = "Alice", .age = 30 };
    people_t obj3 = { .name = "Bob", .age = 35 };
    people_t obj4 = { .name = "aaa", .age = 35 };
    people_t obj5 = { .name = "bbb", .age = 35 };

    cdk_ringbuf_write(&ring, &obj1, 1);
    cdk_ringbuf_write(&ring, &obj2, 1);
    cdk_ringbuf_write(&ring, &obj3, 1);
    cdk_ringbuf_write(&ring, &obj4, 1);
    cdk_ringbuf_write(&ring, &obj5, 1);

    printf("ringbuffer len: %u\n", cdk_ringbuf_len(&ring));

    people_t arr[2];
    cdk_ringbuf_read(&ring, arr, 2);

    for (int i = 0; i < 2; i++) {
        printf("name: %s, age: %d\n", arr[i].name, arr[i].age);
    }
    printf("ringbuffer avail: %u\n", cdk_ringbuf_avail(&ring));
    free(buf);
    return 0;
}
