#include "cdk.h"

atomic_int i;

void callback(void* p)
{
	atomic_fetch_add(&i, 1);
	cdk_logi("exec timer task: %d\n", atomic_load(&i));
}

int main(void) 
{
	cdk_timer_create(cdk_utils_cpus());
	cdk_logger_create(NULL, 0);
	atomic_init(&i, 0);

	for (int i = 0; i < 10; i++) {
		cdk_timer_add(callback, NULL, 2000, true);
	}
	while (1) {
		cdk_time_sleep(1000);
	}
	cdk_timer_destroy();
	cdk_logger_destroy();
	return 0;
}