#include "cdk.h"


int routine(void* p) {

	thrd_t t = thrd_current();
	return 0;
}
int main(void) {

	thrd_t t;
	thrd_create(&t, routine, NULL);
	return 0;
}