#include "cdk.h"

int i;
mtx_t mtx;

int routine(void* param) {

	while (true) {
		mtx_lock(&mtx);
		++i;
		cdk_logi("sub thread output: %d\n", i);
		mtx_unlock(&mtx);

		cdk_time_sleep(100);
	}
	return 0;
}

int main(void) 
{
	cdk_logger_create(NULL, 4);
	thrd_t tid;
	mtx_init(&mtx, mtx_plain);
	thrd_create(&tid, routine, NULL);
	thrd_detach(tid);
	
	while (1) {
		mtx_lock(&mtx);
		++i;
		cdk_logi("main thread output: %d\n", i);
		mtx_unlock(&mtx);
		cdk_time_sleep(100);
	}
	mtx_destroy(&mtx);
	cdk_logger_destroy();
	return 0;
}