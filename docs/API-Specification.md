---
html:
    toc: true
print_background: true
--- 
<center>
Copyright©2023 jin.wu. All rights reserved.
</center>

# CDK API Specification
## Container
### cdk-list
```c
/**
 * @brief Get the data pointer associated with a list node
 *
 * This function retrieves the data pointer associated with a list node `x`.
 * The type of the data pointer is specified by the template parameter `T`.
 * The member variable or field `m` is used to access the data pointer within
 * the list node structure.
 *
 * @param x Pointer to the list node
 * @param T Data type of the data pointer
 * @param m Member variable or field name to access the data pointer
 * @return Pointer to the data associated with the list node
 */
extern T* cdk_list_data(cdk_list_node_t* x, T, m)
```
```c
/**
 * @brief Initialize a doubly linked list
 *
 * This function initializes a doubly linked list by setting it to an empty state.
 *
 * @param l Pointer to the list structure
 * @return N/A
 */
extern void cdk_list_init(cdk_list_t* l)
```
```c
/**
 * @brief Insert a node at the head of the list
 *
 * This function inserts a node `x` at the beginning of the list `l`.
 *
 * @param l Pointer to the list structure
 * @param x Pointer to the node to be inserted
 * @return N/A
 */
extern void cdk_list_insert_head(cdk_list_t* l, cdk_list_node_t* x)
```
```c
/**
 * @brief Insert a node at the tail of the list
 *
 * This function inserts a node `x` at the end of the list `l`.
 *
 * @param l Pointer to the list structure
 * @param x Pointer to the node to be inserted
 * @return N/A
 */
extern void cdk_list_insert_tail(cdk_list_t* l, cdk_list_node_t* x)
```
```c
/**
 * @brief Check if the list is empty
 *
 * This function checks whether the list `l` is empty or not.
 *
 * @param l Pointer to the list structure
 * @return `true` if the list is empty, `false` otherwise
 */
extern bool cdk_list_empty(cdk_list_t* l)
```
```c
/**
 * @brief Get the head node of the list
 *
 * This function returns a pointer to the head node of the list `l`.
 *
 * @param l Pointer to the list structure
 * @return Pointer to the head node of the list
 */
extern cdk_list_node_t* cdk_list_head(cdk_list_t* l)
```
```c
/**
 * @brief Get the tail node of the list
 *
 * This function returns a pointer to the tail node of the list `l`.
 *
 * @param l Pointer to the list structure
 * @return Pointer to the tail node of the list
 */
extern cdk_list_node_t* cdk_list_tail(cdk_list_t* l)
```
```c
/**
 * @brief Get the next node in the list
 *
 * This function returns the next node in the list after the specified node `x`.
 *
 * @param x Pointer to the current node
 * @return Pointer to the next node in the list
 */
extern cdk_list_node_t* cdk_list_next(cdk_list_node_t* x)
```
```c
/**
 * @brief Get the previous node in the list
 *
 * This function returns the previous node in the list before the specified node `x`.
 *
 * @param x Pointer to the current node
 * @return Pointer to the previous node in the list
 */
extern cdk_list_node_t* cdk_list_prev(cdk_list_node_t* x)
```
```c
/**
 * @brief Remove a node from the list
 *
 * This function removes the node `x` from the list.
 *
 * @param x Pointer to the node to be removed
 * @return N/A
 */
extern void cdk_list_remove(cdk_list_node_t* x)
```
```c
/**
 * @brief Get the sentinel node of the list
 *
 * This function returns a pointer to the sentinel node of the list `l`.
 * The sentinel node acts as a special node that represents the list itself.
 *
 * @param l Pointer to the list structure
 * @return Pointer to the sentinel node of the list
 */
extern cdk_list_node_t* cdk_list_sentinel(cdk_list_t* l)
```
### cdk-queue
```c
/**
 * @brief Get the data pointer associated with a queue node
 *
 * This function retrieves the data pointer associated with a queue node `x`.
 * The type of the data pointer is specified by the template parameter `T`.
 * The member variable or field `m` is used to access the data pointer within
 * the queue node structure.
 *
 * @param x Pointer to the queue node
 * @param T Data type of the data pointer
 * @param m Member variable or field name to access the data pointer
 * @return Pointer to the data associated with the queue node
 */
extern T* cdk_queue_data(cdk_queue_node_t* x, T, m)
```
```c
/**
 * @brief Initialize a queue
 *
 * This function initializes a queue by setting it to an empty state.
 *
 * @param q Pointer to the queue structure
 * @return N/A
 */
extern void cdk_queue_init(cdk_queue_t* q);
```
```c
/**
 * @brief Enqueue a node into the queue
 *
 * This function adds a node `x` to the end of the queue `q`.
 *
 * @param q Pointer to the queue structure
 * @param x Pointer to the node to be enqueued
 * @return N/A
 */
extern void cdk_queue_enqueue(cdk_queue_t* q, cdk_queue_node_t* x);
```
```c
/**
 * @brief Dequeue a node from the queue
 *
 * This function removes and returns the node at the front of the queue `q`.
 * If the queue is empty, it returns NULL.
 *
 * @param q Pointer to the queue structure
 * @return Pointer to the dequeued node, or NULL if the queue is empty
 */
extern cdk_queue_node_t* cdk_queue_dequeue(cdk_queue_t* q);
```
```c
/**
 * @brief Check if the queue is empty
 *
 * This function checks whether the queue `q` is empty or not.
 *
 * @param q Pointer to the queue structure
 * @return `true` if the queue is empty, `false` otherwise
 */
extern bool cdk_queue_empty(cdk_queue_t* q);
```
### cdk-rbtree
```c
/**
 * @brief Initialize a red-black tree
 *
 * This function initializes a red-black tree with the specified comparison function.
 *
 * Note: CDK provides the following built-in functions for keycmp：
    extern int default_keycmp_i8(cdk_rbtree_node_key_t* k1, cdk_rbtree_node_key_t* k2);
    extern int default_keycmp_i16(cdk_rbtree_node_key_t* k1, cdk_rbtree_node_key_t* k2);
    extern int default_keycmp_i32(cdk_rbtree_node_key_t* k1, cdk_rbtree_node_key_t* k2);
    extern int default_keycmp_i64(cdk_rbtree_node_key_t* k1, cdk_rbtree_node_key_t* k2);
    extern int default_keycmp_u8(cdk_rbtree_node_key_t* k1, cdk_rbtree_node_key_t* k2);
    extern int default_keycmp_u16(cdk_rbtree_node_key_t* k1, cdk_rbtree_node_key_t* k2);
    extern int default_keycmp_u32(cdk_rbtree_node_key_t* k1, cdk_rbtree_node_key_t* k2);
    extern int default_keycmp_u64(cdk_rbtree_node_key_t* k1, cdk_rbtree_node_key_t* k2);
    extern int default_keycmp_str(cdk_rbtree_node_key_t* k1, cdk_rbtree_node_key_t* k2);
 *
 * @param tree    Pointer to the red-black tree structure
 * @param keycmp  Comparison function for the keys in the tree
 * @return N/A
 */
extern void cdk_rbtree_init(cdk_rbtree_t* tree, int(*keycmp)(cdk_rbtree_node_key_t*, cdk_rbtree_node_key_t*));
```
```c
/**
 * @brief Insert a node into the red-black tree
 *
 * This function inserts a node into the red-black tree `tree`.
 *
 * @param tree  Pointer to the red-black tree structure
 * @param node  Pointer to the node to be inserted
 * @return N/A
 */
extern void cdk_rbtree_insert(cdk_rbtree_t* tree, cdk_rbtree_node_t* node);
```
```c
/**
 * @brief Find a node in the red-black tree
 *
 * This function searches for a node with the specified key 
 * in the red-black tree `tree`.
 * If found, it returns a pointer to the node; otherwise, it returns NULL.
 *
 * @param tree  Pointer to the red-black tree structure
 * @param key   Key of the node to be found
 * @return Pointer to the found node, or NULL if not found
 */
extern cdk_rbtree_node_t* cdk_rbtree_find(cdk_rbtree_t* tree, cdk_rbtree_node_key_t key);
```
```c
/**
 * @brief Erase a node from the red-black tree
 *
 * This function erases a node from the red-black tree `tree`.
 *
 * @param tree  Pointer to the red-black tree structure
 * @param node  Pointer to the node to be erased
 * @return N/A
 */
extern void cdk_rbtree_erase(cdk_rbtree_t* tree, cdk_rbtree_node_t* node);
```
```c
/**
 * @brief Get the next node in the red-black tree
 *
 * This function returns the next node in the red-black tree 
 * after the specified `node`.
 *
 * @param node  Pointer to the current node
 * @return Pointer to the next node in the red-black tree
 */
extern cdk_rbtree_node_t* cdk_rbtree_next(cdk_rbtree_node_t* node);
```
```c
/**
 * @brief Get the previous node in the red-black tree
 *
 * This function returns the previous node in the red-black tree 
 * before the specified `node`.
 *
 * @param node  Pointer to the current node
 * @return Pointer to the previous node in the red-black tree
 */
extern cdk_rbtree_node_t* cdk_rbtree_prev(cdk_rbtree_node_t* node);
```
```c
/**
 * @brief Get the first node in the red-black tree
 *
 * This function returns the first (smallest) node in the red-black tree `tree`.
 *
 * @param tree  Pointer to the red-black tree structure
 * @return Pointer to the first node in the red-black tree
 */
extern cdk_rbtree_node_t* cdk_rbtree_first(cdk_rbtree_t* tree);
```
```c
/**
 * @brief Get the last node in the red-black tree
 *
 * This function returns the last (largest) node in the red-black tree `tree`.
 *
 * @param tree  Pointer to the red-black tree structure
 * @return Pointer to the last node in the red-black tree
 */
extern cdk_rbtree_node_t* cdk_rbtree_last(cdk_rbtree_t* tree);
```
```c
/**
 * @brief Check if the red-black tree is empty
 *
 * This function checks whether the red-black tree `tree` is empty or not.
 *
 * @param tree  Pointer to the red-black tree structure
 * @return `true` if the red-black tree is empty, `false` otherwise
 */
extern bool cdk_rbtree_empty(cdk_rbtree_t* tree);
```
### cdk-ringbuffer
```c
/**
 * @brief Create a ring buffer
 *
 * This function creates a ring buffer with the specified element size 
 * and buffer size.
 *
 * @param ring     Pointer to the ring buffer structure
 * @param esize    Size of each element in the buffer (in bytes)
 * @param bufsize  Total size of the buffer (in bytes)
 * @return N/A
 */
extern void cdk_ringbuf_create(cdk_ringbuf_t* ring, uint32_t esize, uint32_t bufsize);
```
```c
/**
 * @brief Destroy a ring buffer
 *
 * This function destroys a ring buffer, releasing any allocated resources.
 *
 * @param ring  Pointer to the ring buffer structure
 * @return N/A
 */
extern void cdk_ringbuf_destroy(cdk_ringbuf_t* ring);
```
```c
/**
 * @brief Get the current number of elements in the ring buffer
 *
 * This function returns the current number of elements stored in the ring buffer.
 *
 * @param ring  Pointer to the ring buffer structure
 * @return Number of elements in the ring buffer
 */
extern uint32_t cdk_ringbuf_len(cdk_ringbuf_t* ring);
```
```c
/**
 * @brief Get the capacity of the ring buffer
 *
 * This function returns the total capacity (maximum number of elements) 
 * of the ring buffer.
 *
 * @param ring  Pointer to the ring buffer structure
 * @return Capacity of the ring buffer
 */
extern uint32_t cdk_ringbuf_cap(cdk_ringbuf_t* ring);
```
```c
/**
 * @brief Get the number of available (free) elements in the ring buffer
 *
 * This function returns the number of available (free) elements in the ring buffer.
 *
 * @param ring  Pointer to the ring buffer structure
 * @return Number of available elements in the ring buffer
 */
extern uint32_t cdk_ringbuf_avail(cdk_ringbuf_t* ring);
```
```c
/**
 * @brief Check if the ring buffer is full
 *
 * This function checks whether the ring buffer is full or not.
 *
 * @param ring  Pointer to the ring buffer structure
 * @return `true` if the ring buffer is full, `false` otherwise
 */
extern bool cdk_ringbuf_full(cdk_ringbuf_t* ring);
```
```c
/**
 * @brief Check if the ring buffer is empty
 *
 * This function checks whether the ring buffer is empty or not.
 *
 * @param ring  Pointer to the ring buffer structure
 * @return `true` if the ring buffer is empty, `false` otherwise
 */
extern bool cdk_ringbuf_empty(cdk_ringbuf_t* ring);
```
```c
/**
 * @brief Write data to the ring buffer
 *
 * This function writes `entry_count` elements from the provided buffer `buf` 
 * into the ring buffer.
 * It returns the actual number of elements written.
 *
 * @param ring          Pointer to the ring buffer structure
 * @param buf           Pointer to the buffer containing the data to be written
 * @param entry_count   Number of elements to write from the buffer
 * @return Number of elements actually written to the ring buffer
 */
extern uint32_t cdk_ringbuf_write(cdk_ringbuf_t* ring, const void* buf, uint32_t entry_count);
```
```c
/**
 * @brief Read data from the ring buffer
 *
 * This function reads up to `entry_count` elements from the ring buffer 
 * and stores them in the provided buffer `buf`.
 * It returns the actual number of elements read.
 *
 * @param ring          Pointer to the ring buffer structure
 * @param buf           Pointer to the buffer to store the read data
 * @param entry_count   Maximum number of elements to read into the buffer
 * @return Number of elements actually read from the ring buffer
 */
extern uint32_t cdk_ringbuf_read(cdk_ringbuf_t* ring, void* buf, uint32_t entry_count);
```
### cdk-stack
```c
/**
 * @brief Get the data pointer associated with a stack node
 *
 * This function retrieves the data pointer associated with a stack node `x`.
 * The type of the data pointer is specified by the template parameter `T`.
 * The member variable or field `m` is used to access the data pointer within
 * the stack node structure.
 *
 * @param x Pointer to the stack node
 * @param T Data type of the data pointer
 * @param m Member variable or field name to access the data pointer
 * @return Pointer to the data associated with the stack node
 */
extern T* cdk_stack_data(cdk_stack_node_t* x, T, m)
```
```c
/**
 * @brief Initialize a stack
 *
 * This function initializes a stack by setting it to an empty state.
 *
 * @param s Pointer to the stack to be initialized
 * @return N/A
 */
extern void cdk_stack_init(cdk_stack_t* s);
```
```c
/**
 * @brief Push an element onto the stack
 *
 * This function pushes a node `x` onto the stack `s`.
 *
 * @param s Pointer to the stack
 * @param x Pointer to the node to be pushed onto the stack
 * @return N/A
 */
extern void cdk_stack_push(cdk_stack_t* s, cdk_stack_node_t* x);
```
```c
/**
 * @brief Pop an element from the stack
 *
 * This function removes and returns the top node from the stack `s`.
 *
 * @param s Pointer to the stack
 * @return Pointer to the top node that was removed from the stack
 */
extern cdk_stack_node_t* cdk_stack_pop(cdk_stack_t* s);
```
```c
/**
 * @brief Check if the stack is empty
 *
 * This function checks whether the stack `s` is empty or not.
 *
 * @param s Pointer to the stack
 * @return `true` if the stack is empty, `false` otherwise
 */
extern bool cdk_stack_empty(cdk_stack_t* s);
```
## Crypto
### cdk-sha1
```c
/**
 * @brief Initialize the SHA-1 context
 *
 * This function initializes the SHA-1 context to prepare for subsequent 
 * SHA-1 calculations.
 *
 * @param ctx Pointer to the SHA-1 context
 * @return N/A
 */
extern void cdk_sha1_init(cdk_sha1_t* ctx);
```
```c
/**
 * @brief Update the SHA-1 context
 *
 * This function adds data to the SHA-1 context for subsequent SHA-1 calculations.
 *
 * @param ctx  Pointer to the SHA-1 context
 * @param data Pointer to the data to be added to the context
 * @param len  Length of the data to be added to the context (in bytes)
 * @return N/A
 */
extern void cdk_sha1_update(cdk_sha1_t* ctx, uint8_t* data, size_t len);
```
```c
/**
 * @brief Finalize the SHA-1 calculation and obtain the digest
 *
 * This function completes the SHA-1 calculation and retrieves 
 * the final digest (20 bytes).
 *
 * @param ctx    Pointer to the SHA-1 context
 * @param digest Array to store the computed SHA-1 digest (20 bytes)
 * @return N/A
 */
extern void cdk_sha1_final(cdk_sha1_t* ctx, uint8_t digest[20]);
```
### cdk-sha256
```c
/**
 * @brief Initialize the SHA-256 context
 *
 * This function initializes the SHA-256 context to prepare for subsequent 
 * SHA-256 calculations.
 *
 * @param ctx Pointer to the SHA-256 context
 * @return N/A
 */
extern void cdk_sha256_init(cdk_sha256_t* ctx);
```
```c
/**
 * @brief Update the SHA-256 context
 *
 * This function adds data to the SHA-256 context for subsequent 
 * SHA-256 calculations.
 *
 * @param ctx  Pointer to the SHA-256 context
 * @param data Pointer to the data to be added to the context
 * @param len  Length of the data to be added to the context (in bytes)
 * @return N/A
 */
extern void cdk_sha256_update(cdk_sha256_t* ctx, uint8_t* data, size_t len);
```
```c
/**
 * @brief Finalize the SHA-256 calculation and obtain the digest
 *
 * This function completes the SHA-256 calculation and retrieves 
 * the final digest (32 bytes).
 *
 * @param ctx    Pointer to the SHA-256 context
 * @param digest Array to store the computed SHA-256 digest (32 bytes)
 * @return N/A
 */
extern void cdk_sha256_final(cdk_sha256_t* ctx, uint8_t digest[32]);
```
## Encoding
### cdk-base64
```c
/**
 * @brief Encode data using Base64 encoding
 *
 * This function encodes the data specified by `src` using Base64 encoding.
 * The encoded data is stored in the `dst` buffer. The length of the source
 * data is specified by `srclen`, and the length of the encoded data is
 * stored in the `dstlen` variable.
 *
 * @param src     Pointer to the source data to be encoded
 * @param srclen  Length of the source data (in bytes)
 * @param dst     Pointer to the buffer to store the encoded data
 * @param dstlen  Pointer to a variable to store the length of the encoded data
 * @return N/A
 */
extern void cdk_base64_encode(uint8_t* src, size_t srclen, uint8_t* dst, size_t* dstlen);
```
```c
/**
 * @brief Decode data using Base64 decoding
 *
 * This function decodes the Base64-encoded data specified by `src`.
 * The decoded data is stored in the `dst` buffer. The length of the
 * source data is specified by `srclen`, and the length of the decoded
 * data is stored in the `dstlen` variable.
 *
 * @param src     Pointer to the Base64-encoded data to be decoded
 * @param srclen  Length of the source data (in bytes)
 * @param dst     Pointer to the buffer to store the decoded data
 * @param dstlen  Pointer to a variable to store the length of the decoded data
 * @return N/A
 */
extern void cdk_base64_decode(uint8_t* src, size_t srclen, uint8_t* dst, size_t* dstlen);
```
### cdk-varint
```c
/**
 * @brief Encode an unsigned integer as a variable-length integer
 *
 * This function encodes the given unsigned integer `value` as 
 * a variable-length integer and stores the encoded representation 
 * in the buffer `buf`. The buffer must have enough space 
 * to accommodate the encoded value.
 *
 * @param value The unsigned integer value to encode
 * @param buf   Pointer to the buffer to store the encoded value
 * @return The number of bytes written to the buffer
 */
extern int cdk_varint_encode(uint64_t value, char* buf);
```
```c
/**
 * @brief Decode a variable-length integer into an unsigned integer
 *
 * This function decodes a variable-length integer from the buffer `buf` starting at
 * the specified position `pos`. The position is updated to reflect 
 * the next byte after the decoded value. The decoded unsigned integer value 
 * is returned.
 *
 * @param buf Pointer to the buffer containing the encoded value
 * @param pos Pointer to the current position in the buffer (updated after decoding)
 * @return The decoded unsigned integer value
 */
extern uint64_t cdk_varint_decode(char* buf, int* pos);
```
## Net
### cdk-net

## Sync
### cdk-rwlock
### cdk-spinlock


## Others
### cdk-loader
### cdk-logger
### cdk-process
### cdk-threadpool
### cdk-time
### cdk-timer
### cdk-utils