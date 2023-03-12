#include "cdk.h"
#include <inttypes.h>

cdk_mtx_t mtx;
int i;

int routine(void* param) {

	while (true) {
		//cdk_logi("routine thread output\n");
		//uint64_t start = cdk_time_now();
		//cdk_time_sleep(1000);
		//uint64_t end = cdk_time_now();
		cdk_mtx_lock(&mtx);
		cdk_logi("exec logger task: %d\n", i++);
		cdk_mtx_unlock(&mtx);
	}
	return 0;
}

int main(void) {
	cdk_logger_create(NULL, 3);
	cdk_mtx_init(&mtx);

	cdk_thrd_t tid;
	cdk_thrd_create(&tid, routine, NULL);
	cdk_thrd_detach(tid);
	
	while (1) {
		//cdk_logi("main thread output\n");
		cdk_time_sleep(100);
	}
	cdk_mtx_destroy(&mtx);
	cdk_logger_destroy();
	return 0;
}