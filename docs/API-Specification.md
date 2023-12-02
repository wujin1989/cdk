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
```c
/**
 * @brief Convert a network address to presentation format
 *
 * This function converts a network address in `struct sockaddr_storage` format
 * to presentation format and stores the result in the `cdk_addrinfo_t` structure.
 *
 * @param ss Pointer to the `struct sockaddr_storage` containing the network address
 * @param ai Pointer to the `cdk_addrinfo_t` structure to store the converted
 * address
 * @return N/A
 */
extern void cdk_net_ntop(struct sockaddr_storage* ss, cdk_addrinfo_t* ai);
```
```c
/**
 * @brief Convert a network address from presentation format to binary form
 *
 * This function converts a network address in presentation format from 
 * the `cdk_addrinfo_t` structure to binary form and stores the result in the `struct sockaddr_storage`.
 *
 * @param ai Pointer to the `cdk_addrinfo_t` structure containing the network address
 * @param ss Pointer to the `struct sockaddr_storage` to store the converted address
 * @return N/A
 */
extern void cdk_net_pton(cdk_addrinfo_t* ai, struct sockaddr_storage* ss);
```
```c
/**
 * @brief Obtain the local or peer address associated with a socket
 *
 * This function obtains the local or peer address associated with 
 * the specified socket `sock` and stores it in the `cdk_addrinfo_t` structure `ai`. 
 * The `peer` flag indicates whether the peer address should be obtained 
 * (if `true`) or the local address (if `false`).
 *
 * @param sock Socket descriptor
 * @param ai   Pointer to the `cdk_addrinfo_t` structure to  store the obtained address
 * @param peer Flag indicating whether to obtain the peer address (`true`) or local address (`false`)
 * @return N/A
 */
extern void cdk_net_obtain_addr(cdk_sock_t sock, cdk_addrinfo_t* ai, bool peer);
```
```c
/**
 * @brief Get the address family of a socket
 *
 * This function returns the address family (e.g., AF_INET, AF_INET6) of the specified socket `sock`.
 *
 * @param sock Socket descriptor
 * @return The address family of the socket
 */
extern int cdk_net_af(cdk_sock_t sock);
```
```c
/**
 * @brief Get the socket type of a socket
 *
 * This function returns the socket type (e.g., SOCK_STREAM, SOCK_DGRAM) of the specified socket `sock`.
 *
 * @param sock Socket descriptor
 * @return The socket type of the socket
 */
extern int cdk_net_socktype(cdk_sock_t sock);
```
```c
/**
 * @brief Set the receive buffer size of a socket
 *
 * This function sets the receive buffer size of the specified socket `sock` to the specified value `val`.
 *
 * @param sock Socket descriptor
 * @param val  The value of the receive buffer size to set
 * @return N/A
 */
extern void cdk_net_set_recvbuf(cdk_sock_t sock, int val);
```
```c
/**
 * @brief Set the send buffer size of a socket
 *
 * This function sets the send buffer size of the specified socket `sock` to the specified value `val`.
 *
 * @param sock Socket descriptor
 * @param val  The value of the send buffer size to set
 * @return N/A
 */
extern void cdk_net_set_sendbuf(cdk_sock_t sock, int val);
```
```c
/**
 * @brief Listen for incoming connections on a network address
 *
 * This function listens for incoming connections on the specified network address `host:port`
 * of the specified `type`. It creates a channel and associates it with the provided `handler`.
 *
 * @param type    The network address type (e.g., "tcp", "udp")
 * @param host    The host address to listen on
 * @param port    The port number to listen on
 * @param handler Pointer to the handler function for incoming events on the channel
 * @return N/A
 */
extern void cdk_net_listen(const char* type, const char* host, const char* port, cdk_handler_t* handler);
```
```c
/**
 * @brief Establish a network connection to a remote host
 *
 * This function establishes a network connection to the remote host specified by the `host:port`
 * address of the specified `type`. It creates a channel and associates it with the provided `handler`.
 *
 * @param type    The network address type (e.g., "tcp", "udp")
 * @param host    The remote host address to connect to
 * @param port    The port number to connect to
 * @param handler Pointer to the handler function for incoming events on the channel
 * @return N/A
 */
extern void cdk_net_dial(const char* type, const char* host, const char* port, cdk_handler_t* handler);
```
```c
/**
 * @brief Post a receive operation for a channel
 *
 * This function posts a receive operation for the specified channel. It indicates that the channel
 * is ready to receive data.
 *
 * @param channel Pointer to the channel to post the receive operation on
 * @return N/A
 */
extern void cdk_net_postrecv(cdk_channel_t* channel);
```
```c
/**
 * @brief Send buffer by channel
 *
 * This function send buffer by the specified channel.
 *
 * @param channel Pointer to the channel
 * @param data    Pointer to the data to be sent
 * @param size    Size of the data to be sent
 * @return N/A
 */
extern void cdk_net_send(cdk_channel_t* channel, void* data, size_t size);
```
```c
/**
 * @brief Post a event to a poller
 *
 * This function posts a event to the specified poller. The event will be processed by the
 * corresponding event handler.
 *
 * @param poller Pointer to the poller to post the event to
 * @param event  Pointer to the network event to be posted
 * @return N/A
 */
extern void cdk_net_postevent(cdk_poller_t* poller, cdk_event_t* event);
```
```c
/**
 * @brief Close a network channel
 *
 * This function closes the specified network channel and releases associated resources.
 *
 * @param channel Pointer to the channel to be closed
 * @return N/A
 */
extern void cdk_net_close(cdk_channel_t* channel);
```
```c
/**
 * @brief Initialize a network unpacker for a channel
 *
 * This function initializes a network unpacker for the specified channel. The unpacker is used
 * to unpack received data into meaningful messages or packets.
 *
 * @param channel  Pointer to the channel to initialize the unpacker for
 * @param unpacker Pointer to the unpacker structure to be initialized
 * @return N/A
 */
extern void cdk_net_unpacker_init(cdk_channel_t* channel, cdk_unpack_t* unpacker);
```
```c
/**
 * @brief Start up the network system with worker threads and TLS configuration
 *
 * This function starts up the network system with the specified number of worker threads and
 * the provided TLS configuration. It prepares the system to handle network operations.
 *
 * @param nworkers  Number of worker threads to start
 * @param tlsconf   Pointer to the TLS configuration for secure network communication (can be NULL for non-secure communication)
 * @return N/A
 */
extern void cdk_net_startup(int nworkers, cdk_tlsconf_t* tlsconf);
```
```c
/**
 * @brief Clean up and shut down the network system
 *
 * This function cleans up and shuts down the network system. It releases all associated resources
 * and terminates the network operations.
 * 
 * @param N/A
 * @return N/A
 */
extern void cdk_net_cleanup(void);
```
## Sync
### cdk-rwlock
```c
/**
 * @brief Initialize a reader-writer lock
 *
 * This function initializes a reader-writer lock by setting its initial state.
 *
 * @param rwlock Pointer to the reader-writer lock to be initialized
 * @return N/A
 */
extern void cdk_rwlock_init(cdk_rwlock_t* rwlock);
```
```c
/**
 * @brief Acquire a read lock
 *
 * This function acquires a read lock on the specified reader-writer lock. Multiple threads can
 * acquire the read lock simultaneously, allowing concurrent read access to a shared resource.
 *
 * @param rwlock Pointer to the reader-writer lock to acquire the read lock on
 * @return N/A
 */
extern void cdk_rwlock_rdlock(cdk_rwlock_t* rwlock);
```
```c
/**
 * @brief Acquire a write lock
 *
 * This function acquires a write lock on the specified reader-writer lock. Only one thread can
 * acquire the write lock at a time, ensuring exclusive write access to a shared resource.
 *
 * @param rwlock Pointer to the reader-writer lock to acquire the write lock on
 * @return N/A
 */
extern void cdk_rwlock_wrlock(cdk_rwlock_t* rwlock);
```
```c
/**
 * @brief Release a read lock
 *
 * This function releases a previously acquired read lock on the specified reader-writer lock.
 * It allows other threads to acquire the read lock and access the shared resource.
 *
 * @param rwlock Pointer to the reader-writer lock to release the read lock from
 * @return N/A
 */
extern void cdk_rwlock_rdunlock(cdk_rwlock_t* rwlock);
```
```c
/**
 * @brief Release a write lock
 *
 * This function releases a previously acquired write lock on the specified reader-writer lock.
 * It allows other threads to acquire the write lock and access the shared resource.
 *
 * @param rwlock Pointer to the reader-writer lock to release the write lock from
 * @return N/A
 */
extern void cdk_rwlock_wrunlock(cdk_rwlock_t* rwlock);
```
### cdk-spinlock
```c
/**
 * @brief Initialize a spinlock
 *
 * This function initializes a spinlock by setting its initial state.
 *
 * @param lock Pointer to the spinlock to be initialized
 * @return N/A
 */
extern void cdk_spinlock_init(cdk_spinlock_t* lock);
```
```c
/**
 * @brief Acquire a spinlock
 *
 * This function acquires a spinlock, which is a simple form of lock that uses busy waiting. It
 * ensures exclusive access to a shared resource by blocking other threads from acquiring the lock
 * until it is released.
 *
 * @param lock Pointer to the spinlock to be acquired
 * @return N/A
 */
extern void cdk_spinlock_lock(cdk_spinlock_t* lock);
```
```c
/**
 * @brief Release a spinlock
 *
 * This function releases a previously acquired spinlock, allowing other threads to acquire the
 * lock and access the shared resource.
 *
 * @param lock Pointer to the spinlock to be released
 * @return N/A
 */
extern void cdk_spinlock_unlock(cdk_spinlock_t* lock);
```
## Others
### cdk-loader
```c
/**
 * @brief Create a loader
 *
 * This function creates a loader by initializing its internal state.
 *
 * @param m Name of the module or library to load
 * @return Pointer to the created loader
 */
extern void* cdk_loader_create(char* m);
```
```c
/**
 * @brief Load a function using the loader
 *
 * This function loads a function specified by its name using the given loader. It retrieves the
 * function's address or entry point for later use.
 *
 * @param m Pointer to the loader
 * @param f Name of the function to load
 * @return Pointer to the loaded function
 */
extern void* cdk_loader_load(void* m, const char* restrict f);
```
```c
/**
 * @brief Destroy a loader
 *
 * This function destroys a loader and releases any associated resources.
 *
 * @param m Pointer to the loader to be destroyed
 * @return N/A
 */
extern void  cdk_loader_destroy(void* m);
```
### cdk-logger
```c
/**
 * @brief Create a logger
 *
 * This function creates a logger and initializes its internal state. It specifies the output destination
 * for log messages and the number of threads to be used for logging.
 *
 * @param out The output destination for log messages
 * @param nthrds The number of threads to be used for logging
 * @return N/A
 */
extern void cdk_logger_create(const char* restrict out, int nthrds);
```
```c
/**
 * @brief Destroy a logger
 *
 * This function destroys a logger and releases any associated resources. It should be called when the
 * logger is no longer needed.
 * @param N/A
 * @return N/A
 */
extern void cdk_logger_destroy(void);
```
```c
/**
 * @brief Log an informational message
 *
 * This function logs an informational message with the provided format and optional arguments.
 * It is typically used to provide informative and non-critical messages during program execution.
 *
 * @param format The format string for the log message
 * @param ... Optional arguments to be formatted and included in the log message
 * @return N/A
 */
extern void cdk_logi(const char *restrict format, ...);
```
```c
/**
 * @brief Log a debug message
 *
 * This function logs a debug message with the provided format and optional arguments.
 * It is typically used for detailed debugging and troubleshooting purposes during program execution.
 *
 * @param format The format string for the log message
 * @param ... Optional arguments to be formatted and included in the log message
 * @return N/A
 */
extern void cdk_logd(const char *restrict format, ...);
```
```c
/**
 * @brief Log a warning message
 *
 * This function logs a warning message with the provided format and optional arguments.
 * It is typically used to indicate potential issues or non-fatal errors during program execution.
 *
 * @param format The format string for the log message
 * @param ... Optional arguments to be formatted and included in the log message
 * @return N/A
 */
extern void cdk_logw(const char *restrict format, ...);
```
```c
/**
 * @brief Log a warning message
 *
 * This function logs a warning message with the provided format and optional arguments.
 * It is typically used to indicate potential issues or non-fatal errors during program execution.
 *
 * @param format The format string for the log message
 * @param ... Optional arguments to be formatted and included in the log message
 * @return N/A
 */
extern void cdk_loge(const char *restrict format, ...);
```
### cdk-process
```c
/**
 * @brief Get the process ID
 *
 * This function retrieves the process ID (PID) of the current process.
 * The PID is a unique identifier assigned to a process by the operating system.
 *
 * @return The process ID (PID) of the current process
 */
extern cdk_pid_t cdk_process_getpid(void);
```
### cdk-threadpool
```c
/**
 * @brief Create a thread pool
 *
 * This function creates a thread pool with the specified number of threads.
 * A thread pool is a collection of threads that can be used to execute tasks concurrently.
 *
 * @param pool Pointer to the thread pool object to be initialized
 * @param nthrds The number of threads to create in the thread pool
 * @return N/A
 */
extern void cdk_thrdpool_create(cdk_thrdpool_t* pool, int nthrds);
```
```c
/**
 * @brief Post a task to the thread pool
 *
 * This function posts a task, represented by a function pointer and an argument, to the thread pool.
 * The task will be executed by one of the threads in the pool in an asynchronous manner.
 *
 * @param pool Pointer to the thread pool where the task will be submitted
 * @param routine Function pointer to the task routine to be executed
 * @param arg Pointer to the argument to be passed to the task routine
 * @return N/A
 */
extern void cdk_thrdpool_post(cdk_thrdpool_t* pool, void (*routine)(void*), void* arg);
```
```c
/**
 * @brief Destroy a thread pool
 *
 * This function destroys a thread pool, releasing all associated resources.
 * After calling this function, the thread pool should no longer be used.
 *
 * @param pool Pointer to the thread pool object to be destroyed
 * @return N/A
 */
extern void cdk_thrdpool_destroy(cdk_thrdpool_t* pool);
```
### cdk-time
```c
/**
 * @brief Get the current time since the Unix Epoch (UTC)
 *
 * This function returns the current time as the number of milliseconds
 * elapsed since the Unix Epoch, which is defined as midnight (00:00:00 UTC) on
 * January 1, 1970.
 *
 * @return The current time since the Unix Epoch in milliseconds
 */
extern uint64_t cdk_time_now(void);
```
```c
/**
 * @brief Convert a timestamp to local time
 *
 * This function converts the given timestamp to local time and stores the result in the struct tm.
 *
 * @param t Pointer to the timestamp to be converted
 * @param r Pointer to the struct tm to store the converted local time
 * @return N/A
 */
extern void cdk_time_localtime(const time_t* t, struct tm* r);
```
```c
/**
 * @brief Sleep for a specified duration
 *
 * This function suspends the execution of the current thread for the specified number of milliseconds.
 *
 * @param ms The duration to sleep in milliseconds
 * @return N/A
 */
extern void cdk_time_sleep(const uint32_t ms);
```
### cdk-timer
```c
/**
 * @brief Create a timer
 *
 * This function initializes a timer, which can be used to manage and schedule timer jobs.
 *
 * @param timer Pointer to the timer to be created
 * @param nthrds Number of worker threads for the timer
 * @return N/A
 */
extern void cdk_timer_create(cdk_timer_t* timer, int nthrds);
```
```c
/**
 * @brief Destroy a timer
 *
 * This function cleans up and releases the resources associated with a timer object.
 *
 * @param timer Pointer to the timer object to be destroyed
 * @return N/A
 */
extern void cdk_timer_destroy(cdk_timer_t* timer);
```
```c
/**
 * @brief Add a timer job
 *
 * This function adds a timer job to the specified timer. The timer job is scheduled to run after the specified
 * expiration time, and optionally repeat at regular intervals.
 *
 * @param timer Pointer to the timer object
 * @param routine Pointer to the function to be executed by the timer job
 * @param arg Argument to be passed to the timer job function
 * @param expire Expiration time in milliseconds for the timer job
 * @param repeat Flag indicating whether the timer job should repeat after each expiration
 * @return Pointer to the timer job object representing the added job
 */
extern cdk_timer_job_t* cdk_timer_add(cdk_timer_t* timer, void (*routine)(void*), void* arg, uint32_t expire, bool repeat);
```
```c
/**
 * @brief Delete a timer job
 *
 * This function removes a timer job from the specified timer. The timer job will no longer be executed by the timer.
 *
 * @param timer Pointer to the timer object
 * @param job Pointer to the timer job object to be deleted
 * @return N/A
 */
extern void cdk_timer_del(cdk_timer_t* timer, cdk_timer_job_t* job);
```
### cdk-utils
```c
/**
 * @brief Get the byte order of the system
 *
 * This function determines the byte order of the system by checking the endianness.
 * It returns an integer value indicating the byte order of the system.
 * The value is typically used to determine whether the system is little-endian or big-endian.
 *
 * @return An integer representing the byte order of the system:
 *         - 0 if the system is big-endian
 *         - 1 if the system is little-endian
 */
extern int cdk_utils_byteorder(void);
```
```c
/**
 * @brief Get the number of CPUs available on the system
 *
 * This function returns the number of CPUs available on the system.
 *
 * @return An integer representing the number of CPUs on the system
 */
extern int cdk_utils_cpus(void);
```
```c
/**
 * @brief Get the system thread ID
 *
 * This function returns the thread ID of the calling thread.
 *
 * @return The system thread ID
 */
cdk_tid_t cdk_utils_systemtid(void);
```
```c
/**
 * @brief Generate a random integer within the specified range
 *
 * This function generates a random integer within the specified range.
 *
 * @param min The minimum value of the range (inclusive)
 * @param max The maximum value of the range (inclusive)
 * @return The randomly generated integer
 */
extern int cdk_utils_rand(int min, int max);
```
