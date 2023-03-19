#include "cdk.h"
#include <inttypes.h>

//mtx_t mtx;
int i;

int routine(void* param) {

	while (true) {
		//cdk_logi("routine thread output\n");
		//uint64_t start = cdk_time_now();
		//cdk_time_sleep(1000);
		//uint64_t end = cdk_time_now();
		//mtx_lock(&mtx);
		cdk_logi("exec logger task\n");
		//mtx_unlock(&mtx);
	}
	return 0;
}

int main(void) {
	cdk_logger_create(NULL, 0);
	//mtx_init(&mtx, mtx_plain);

	thrd_t tid;
	thrd_create(&tid, routine, NULL);
	thrd_detach(tid);

	thrd_t tid2;
	thrd_create(&tid2, routine, NULL);
	thrd_detach(tid2);
	
	while (1) {
		cdk_logi("main thread output\n");
		//cdk_time_sleep(100);
	}
	//mtx_destroy(&mtx);
	cdk_logger_destroy();
	return 0;
}