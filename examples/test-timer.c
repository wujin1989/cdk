#include "cdk.h"
#include <inttypes.h>

void callback(void* p) {

	static int i;
	cdk_logi("exec timer task: %d\n", i++);
}

int main(void) {

	cdk_timer_create(2);
	cdk_logger_init(NULL, false, 0);
	
	for (int i = 0; i < 100; i++) {
		cdk_timer_add(callback, NULL, 5000, false);
	}
	//cdk_timer_add(&timer, callback, NULL, 2000, true);
	while (1) {
		cdk_time_sleep(1000);
	}
	cdk_timer_destroy();
	cdk_logger_destroy();
	return 0;
}