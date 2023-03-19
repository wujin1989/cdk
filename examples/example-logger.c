#include "cdk.h"
#include <inttypes.h>

int i;

int routine(void* param) {

	while (true) {
		++i;
		cdk_logi("sub thread output: %d\n", i);
		cdk_time_sleep(100);
	}
	return 0;
}

int main(void) 
{
	cdk_logger_create(NULL, 0);   //sync
	//cdk_logger_create(NULL, 2); //async

	thrd_t tid;
	thrd_create(&tid, routine, NULL);
	thrd_detach(tid);

	thrd_t tid2;
	thrd_create(&tid2, routine, NULL);
	thrd_detach(tid2);
	
	while (1) {
		++i;
		cdk_logi("main thread output: %d\n", i);
		cdk_time_sleep(100);
	}
	cdk_logger_destroy();
	return 0;
}