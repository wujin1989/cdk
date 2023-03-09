#include "cdk.h"
#include <inttypes.h>

cdk_mtx_t mtx;
int i;

void callback(void* p) {
	
	cdk_mtx_lock(&mtx);
	cdk_logi("exec timer task: %d\n", i++);
	cdk_mtx_unlock(&mtx);
}

int main(void) {

	cdk_timer_create();
	cdk_logger_create(NULL, true);
	cdk_mtx_init(&mtx);

	for (int i = 0; i < 10; i++) {
		cdk_timer_add(callback, NULL, 2000, true);
	}
	while (1) {
		cdk_time_sleep(1000);
	}
	cdk_mtx_destroy(&mtx);
	cdk_timer_destroy();
	cdk_logger_destroy();
	return 0;
}