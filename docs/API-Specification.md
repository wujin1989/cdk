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
 * Retrieve a pointer to data type T based on the node x of a linked list.
 *
 * @param x, Node of a linked list
 * @param T, Data type
 * @param m, The member name of the linked list node x in the data type T
 * @return Pointer to the data type T
 */
extern T* cdk_list_data(cdk_list_node_t* x, T, m)
```
```c
/**
 * Initialize a linked list.
 * 
 * @param l, Ptr of a linked list
 * @return N/A
 */
extern void cdk_list_init(cdk_list_t* l)
```
```c
/**
 * Insert node x at the head of linked list l.
 * 
 * @param l, Ptr of a linked-list
 * @param x, Ptr of a linked-list node
 * @return N/A
 */
extern void cdk_list_insert_head(cdk_list_t* l, cdk_list_node_t* x)
```
```c
/**
 * Insert node x at the end of linked list l.
 * 
 * @param l, Ptr of a linked-list
 * @param x, Ptr of a linked-list node
 * @return N/A
 */
extern void cdk_list_insert_tail(cdk_list_t* l, cdk_list_node_t* x)
```
```c
/**
 * Check if the linked list is empty.
 * 
 * @param l, Ptr of a linked-list
 * @return True or False
 */
extern bool cdk_list_empty(cdk_list_t* l)
```
```c
/**
 * Returns the first node of the linked list.
 * 
 * @param l, Ptr of a linked-list
 * @return Ptr of first node
 */
extern cdk_list_node_t* cdk_list_head(cdk_list_t* l)
```
```c
/**
 * Returns the last node of the linked list.
 * 
 * @param l, Ptr of a linked-list
 * @return Ptr of last node
 */
extern cdk_list_node_t* cdk_list_tail(cdk_list_t* l)
```
```c
/**
 * Returns the next node of node x.
 * 
 * @param x, Ptr of a linked-list node
 * @return Ptr of next node
 */
extern cdk_list_node_t* cdk_list_next(cdk_list_node_t* x)
```
```c
/**
 * Returns the previous node of node x.
 * 
 * @param x, Ptr of a linked-list node
 * @return Ptr of previous node
 */
extern cdk_list_node_t* cdk_list_prev(cdk_list_node_t* x)
```
```c
/**
 * Remove node x from linked list.
 * 
 * @param x, Ptr of a linked-list node
 * @return N/A
 */
extern void cdk_list_remove(cdk_list_node_t* x)
```
```c
/**
 * Returns the sentinel node of the linked list l.
 * 
 * @param l, Ptr of a linked-list
 * @return Ptr of sentinel node
 */
extern cdk_list_node_t* cdk_list_sentinel(cdk_list_t* l)
```
### cdk-queue
```c
/**
 * Retrieve a pointer to data type T based on the node x of a queue.
 *
 * @param x, Node of a queue
 * @param T, Data type
 * @param m, The member name of the queue node x in the data type T
 * @return Pointer to the data type T
 */
extern T* cdk_queue_data(cdk_queue_node_t* x, T, m)
```
```c
/**
 * Initialize a queue.
 * 
 * @param q, Ptr of a queue
 * @return N/A
 */
extern void cdk_queue_init(cdk_queue_t* q);
```
```c
/**
 * Add node x to queue q.
 * 
 * @param q, Ptr of a queue
 * @param x, Ptr of a queue node
 * @return N/A
 */
extern void cdk_queue_enqueue(cdk_queue_t* q, cdk_queue_node_t* x);
```
```c
/**
 * Remove a node from queue q.
 * 
 * @param q, Ptr of a queue
 * @return The pointer to the deleted queue node
 */
extern cdk_queue_node_t* cdk_queue_dequeue(cdk_queue_t* q);
```
```c
/**
 * Check if the queue is empty.
 * 
 * @param q, Ptr of a queue.
 * @return True or False
 */
extern bool cdk_queue_empty(cdk_queue_t* q);
```
### cdk-rbtree
```c
/**
 * Initialize a red-black tree.
 * 
 * @param tree, Ptr of a red-black tree
 * @param keycmp, Ptr to the comparison function of the red-black tree key
 * @return N/A
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
 */
extern void cdk_rbtree_init(cdk_rbtree_t* tree, int(*keycmp)(cdk_rbtree_node_key_t*, cdk_rbtree_node_key_t*));
```
```c
/**
 * Insert the node into the red-black tree.
 * 
 * @param tree, Ptr of a red-black tree
 * @param node, Ptr of a red-black tree's node
 * @return N/A
 */
extern void cdk_rbtree_insert(cdk_rbtree_t* tree, cdk_rbtree_node_t* node);
```
```c
/**
 * Find the node in the red-black tree according to the key.
 * 
 * @param tree, Ptr of a red-black tree
 * @param key, rbtree node's key
 * @return Ptr of rbtree node
 */
extern cdk_rbtree_node_t* cdk_rbtree_find(cdk_rbtree_t* tree, cdk_rbtree_node_key_t key);
```
```c
/**
 * Deletes the specified node from RBtree.
 * 
 * @param tree, Ptr of a red-black tree
 * @param node, Ptr of a red-black tree's node
 * @return N/A
 */
extern void cdk_rbtree_erase(cdk_rbtree_t* tree, cdk_rbtree_node_t* node);
```
```c
/**
 * Retrieve the next node of the red-black tree based on the specified node.
 * 
 * @param node, Ptr of a red-black tree's node
 * @return The next node pointer of the red-black tree
 */
extern cdk_rbtree_node_t* cdk_rbtree_next(cdk_rbtree_node_t* node);
```
```c
/**
 * Retrieve the previous node of the red-black tree based on the specified node.
 * 
 * @param node, Ptr of a red-black tree's node
 * @return The previous node pointer of the red-black tree
 */
extern cdk_rbtree_node_t* cdk_rbtree_prev(cdk_rbtree_node_t* node);
```
```c
/**
 * Retrieve the first node of the red-black tree.
 * 
 * @param tree, Ptr of a red-black tree
 * @return The first node pointer of the red-black tree
 */
extern cdk_rbtree_node_t* cdk_rbtree_first(cdk_rbtree_t* tree);
```
```c
/**
 * Retrieve the last node of the red-black tree.
 * 
 * @param tree, Ptr of a red-black tree
 * @return The last node pointer of the red-black tree
 */
extern cdk_rbtree_node_t* cdk_rbtree_last(cdk_rbtree_t* tree);
```
```c
/**
 * Check if red-black tree is empty.
 * 
 * @param tree, Ptr of a red-black tree
 * @return True or False
 */
extern bool cdk_rbtree_empty(cdk_rbtree_t* tree);
```
### cdk-ringbuffer
```c
extern void cdk_ringbuf_init(cdk_ringbuf_t* ring, uint32_t esize, void* buf, uint32_t bsize);
```
```c
extern uint32_t cdk_ringbuf_len(cdk_ringbuf_t* ring);
```
```c
extern uint32_t cdk_ringbuf_cap(cdk_ringbuf_t* ring);
```
```c
extern uint32_t cdk_ringbuf_avail(cdk_ringbuf_t* ring);
```
```c
extern bool cdk_ringbuf_full(cdk_ringbuf_t* ring);
```
```c
extern bool cdk_ringbuf_empty(cdk_ringbuf_t* ring);
```
```c
extern uint32_t cdk_ringbuf_write(cdk_ringbuf_t* ring, const void* buf, uint32_t entry_count);
```
```c
extern uint32_t cdk_ringbuf_read(cdk_ringbuf_t* ring, void* buf, uint32_t entry_count);
```
### cdk-stack

## Crypto
### cdk-sha1
### cdk-sha256

## Encoding
### cdk-base64
### cdk-varint
### cdk-json

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