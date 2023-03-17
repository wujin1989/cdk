#include "cdk.h"


int routine(void* p) {

	thrd_t t = thrd_current();
	printf("routine: %p\n", t);
	return 0;
}
int main(void) {

	thrd_t t;
	thrd_create(&t, routine, NULL);
	printf("main: %p\n", t);
	return 0;
}