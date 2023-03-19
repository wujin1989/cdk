#include "cdk.h"

int cnt;
cdk_spinlock_t lock;

int func(void* p) {

	while (true) {
		cdk_spinlock_lock(&lock);
		cdk_logi("sub cnt: %d\n", ++cnt);
		cdk_spinlock_unlock(&lock);

		cdk_time_sleep(2000);
	}

	return 0;
}

int main(void) {

	cdk_logger_create(NULL, 0);
	cdk_spinlock_init(&lock);

	thrd_t t;
	thrd_create(&t, func, NULL);

	while (true) {
		cdk_spinlock_lock(&lock);
		cdk_logi("main cnt: %d\n", ++cnt);
		cdk_spinlock_unlock(&lock);

		cdk_time_sleep(2000);
	}
	thrd_join(t, NULL);
	cdk_logger_destroy();
	return 0;
}