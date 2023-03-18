#include "cdk.h"
#include <stdio.h>

typedef struct userdata_s {
	int value;
	cdk_rbtree_node_t node;
}userdata_t;

int main(void) {

	cdk_rbtree_t tree;
	cdk_rbtree_create(&tree, RB_KEYTYPE_INT32);

	for (int i = 0; i < 10; i++) {
		userdata_t* ud = malloc(sizeof(userdata_t));
		ud->value = i;
		ud->node.rb_key.i32 = i;
		cdk_rbtree_insert(&tree, &ud->node);
	}
	cdk_rbtree_node_key_t key;
	key.i32 = 3;
	cdk_rbtree_node_t* node = cdk_rbtree_find(&tree, key);

	userdata_t* ud = cdk_rbtree_data(node, userdata_t, node);
	printf("before erase, userdata: value=%d\n", ud->value);

	cdk_rbtree_erase(&tree, &ud->node);

	cdk_rbtree_node_t* node2 = cdk_rbtree_find(&tree, key);
	if (node2) {
		userdata_t* ud2 = cdk_rbtree_data(cdk_rbtree_find(&tree, key), userdata_t, node);
		printf("after erase, userdata: value=%d\n", ud2->value);
	}
	cdk_rbtree_node_t* node3 = cdk_rbtree_first(&tree);
	while(node3) {
		userdata_t* ud = cdk_rbtree_data(node3, userdata_t, node);
		printf("userdata: value=%d\n", ud->value);
		cdk_rbtree_erase(&tree, node3);
		node3 = cdk_rbtree_first(&tree);
	}
	return 0;
}