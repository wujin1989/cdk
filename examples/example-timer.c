#include "cdk.h"

mtx_t mtx;

void callback(void* p) {
	mtx_lock(&mtx);
	cdk_logi("[%d]timer task\n", (int)cdk_utils_systemtid());
	mtx_unlock(&mtx);
}

int main(void) {
	cdk_timer_create();
	cdk_logger_create(NULL, true);
	mtx_init(&mtx, mtx_plain);

	cdk_timer_add(callback, NULL, 1000, true);
	
	while (1) {
		cdk_time_sleep(1000);
	}
	mtx_destroy(&mtx);
	cdk_timer_destroy();
	cdk_logger_destroy();
	return 0;
}