#include "cdk.h"

typedef struct userdata_s {
	int value;
	cdk_rbtree_node_t node;
}userdata_t;

int main(void) {
	cdk_rbtree_t tree;
	cdk_rbtree_init(&tree, default_keycmp_i32);

	for (int i = 0; i < 10; i++) {
		userdata_t* ud = malloc(sizeof(userdata_t));
		if (ud) {
			ud->value = i;
			ud->node.rb_key.i32 = i;
			cdk_rbtree_insert(&tree, &ud->node);
		}
	}
	cdk_rbtree_node_key_t key;
	key.i32 = 3;
	userdata_t* ud = cdk_rbtree_data(cdk_rbtree_find(&tree, key), userdata_t, node);
	if (ud) {
		printf("key: %d, value: %d\n", ud->node.rb_key.i32, ud->value);
		cdk_rbtree_erase(&tree, &ud->node);
		free(ud);
		ud = NULL;
	}
	while(!cdk_rbtree_empty(&tree)) {
		userdata_t* ud = cdk_rbtree_data(cdk_rbtree_first(&tree), userdata_t, node);
		if (ud) {
			printf("key: %d, value: %d\n", ud->node.rb_key.i32, ud->value);
			cdk_rbtree_erase(&tree, &ud->node);
			free(ud);
			ud = NULL;
		}
	}
	return 0;
}