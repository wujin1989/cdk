#include "cdk.h"
#include <inttypes.h>

int routine(void* param) {

	while (true) {
		//cdk_logi("routine thread output\n");
		uint64_t start = cdk_time_now();
		cdk_time_sleep(1);
		uint64_t end = cdk_time_now();
		printf("cost %"PRIu64"\n", end - start);
	}
	return 0;
}

int main(void) {

	cdk_logger_init(NULL, true, 4);
	
	cdk_thrd_t tid;
	cdk_thrd_create(&tid, routine, NULL);
	cdk_thrd_detach(tid);
	
	while (1) {
		//cdk_logi("main thread output\n");
		cdk_time_sleep(100);
	}
	cdk_logger_destroy();
	return 0;
}