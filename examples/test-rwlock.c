#include "cdk.h"

int cnt;
cdk_rwlock_t rwlock;

int reader(void* p) {

	while (true) {
		cdk_rwlock_rdlock(&rwlock);
		printf("[%d] read cnt: %d\n", (int)cdk_utils_systemtid(), cnt);
		cdk_rwlock_unlock(&rwlock);

		cdk_time_sleep(2000);
	}
	
	return 0;
}
int writer(void* p) {

	while (true) {
		cdk_rwlock_wrlock(&rwlock);
		printf("[%d] write cnt: %d\n", (int)cdk_utils_systemtid(), ++cnt);
		cdk_rwlock_unlock(&rwlock);

		cdk_time_sleep(4000);
	}
	
	return 0;
}
int main(void) {

	cdk_rwlock_init(&rwlock);

	for (int i = 0; i < 3; i++) {
		thrd_t t;
		thrd_create(&t, reader, NULL);
		thrd_detach(t);
	}
	thrd_t t2;
	thrd_create(&t2, writer, NULL);

	thrd_join(t2, NULL);
	return 0;
}