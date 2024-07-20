#include "cdk.h"

typedef struct udata_s {
	int value;
	cdk_rbtree_node_t node;
}udata_t;

int main(void) {
	cdk_rbtree_t tree;
	cdk_rbtree_init(&tree, default_keycmp_i32);

	for (int i = 0; i < 10; i++) {
		udata_t* ud = malloc(sizeof(udata_t)); assert(ud);
		ud->value = i * 10;
		ud->node.key.i32 = i;
		cdk_rbtree_insert(&tree, &ud->node);
	}
	cdk_rbtree_key_t key = {.i32 = 3};
	udata_t* ud = cdk_rbtree_data(cdk_rbtree_find(&tree, key), udata_t, node); assert(ud);

	printf("K: %d, V: %d\n", ud->node.key.i32, ud->value);
	cdk_rbtree_erase(&tree, &ud->node); free(ud); ud = NULL;
	
	for (cdk_rbtree_node_t* n = cdk_rbtree_first(&tree); ; n = cdk_rbtree_next(n)) {
		udata_t* ud = cdk_rbtree_data(n, udata_t, node); assert(ud);
		printf("K: %d, V: %d\n", ud->node.key.i32, ud->value);
		if (n == cdk_rbtree_last(&tree)) { break; }
	}
	return 0;
}