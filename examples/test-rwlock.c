#include "cdk.h"

int cnt;
cdk_rwlock_t rwlock;

int reader(void* p) {

	while (true) {
		cdk_rwlock_rdlock(&rwlock);
		cdk_logi("[%d] read cnt: %d\n", (int)cdk_utils_systemtid(), cnt);
		cdk_rwlock_rdunlock(&rwlock);
		
		cdk_time_sleep(2000);
	}
	
	return 0;
}
int writer(void* p) {

	while (true) {
		cdk_rwlock_wrlock(&rwlock);
		cdk_logi("[%d] write cnt: %d\n", (int)cdk_utils_systemtid(), ++cnt);
		cdk_rwlock_wrunlock(&rwlock);

		cdk_time_sleep(4000);
	}
	
	return 0;
}
int main(void) {

	cdk_logger_create(NULL, 0);
	cdk_rwlock_init(&rwlock);

	for (int i = 0; i < 5; i++) {
		thrd_t t;
		thrd_create(&t, reader, NULL);
		thrd_create(&t, reader, NULL);
		thrd_detach(t);
	}

	thrd_t t;
	thrd_create(&t, writer, NULL);
	thrd_join(t, NULL);
	cdk_logger_destroy();
	return 0;
}