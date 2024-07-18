#include "cdk.h"

typedef struct udata_s {
	cdk_heap_node_t node;
	int val;
} udata_t;
static int cmp_fn(cdk_heap_node_t* a, cdk_heap_node_t* b) {
	udata_t* m = cdk_heap_data(a, udata_t, node);
	udata_t* n = cdk_heap_data(b, udata_t, node);

	if (m->val < n->val) {
		return 1;
	}
	return 0;
}

int main(void) {
	cdk_heap_t heap;
	cdk_heap_init(&heap, cmp_fn);
	
	for (int i = 0; i < 10; i++) {
		udata_t* ud = malloc(sizeof(udata_t));
		if (ud) {
			ud->val = i;
			cdk_heap_insert(&heap, &ud->node);
		}
	}
	while (!cdk_heap_empty(&heap)) {
		udata_t* ud = cdk_heap_data(cdk_heap_min(&heap), udata_t, node);
		printf("current min udata's value: %d\n", ud->val);
		cdk_heap_dequeue(&heap);
	}
	return 0;
}