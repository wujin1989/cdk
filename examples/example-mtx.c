#include "cdk.h"

int cnt;
mtx_t mtx;

int func1(void* p) {
	while (true) {
		mtx_lock(&mtx);
		cdk_logi("func1: cnt is %d\n", cnt++);
		mtx_unlock(&mtx);
		cdk_time_sleep(2000);
	}
}

int func2(void* p) {
	while (true) {
		mtx_lock(&mtx);
		cdk_logi("func2: cnt is %d\n", cnt++);
		mtx_unlock(&mtx);
		cdk_time_sleep(2000);
	}
}
int main(void) {
	thrd_t t1, t2;
	cdk_logger_create(NULL, false);

	mtx_init(&mtx, mtx_plain);
	thrd_create(&t1, func1, NULL);
	thrd_create(&t2, func2, NULL);

	thrd_join(t1, NULL);
	thrd_join(t2, NULL);

	mtx_destroy(&mtx);
	cdk_logger_destroy();
	return 0;
}