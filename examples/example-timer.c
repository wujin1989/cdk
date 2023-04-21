#include "cdk.h"

int i;
mtx_t mtx;

void callback(void* p)
{
	mtx_lock(&mtx);
	cdk_logi("exec timer task: %d\n", ++i);
	mtx_unlock(&mtx);
}

int main(void) 
{
	cdk_timer_t timer;
	cdk_timer_create(&timer, cdk_utils_cpus());
	cdk_logger_create(NULL, 0);
	mtx_init(&mtx, mtx_plain);
	for (int i = 0; i < 10; i++) {
		cdk_timer_job_t* job = malloc(sizeof(cdk_timer_job_t));
		if (job) {
			job->routine = callback;
			job->arg = NULL;
			job->birthtime = cdk_time_now();
			job->expire = 1000;
			job->repeat = true;

			cdk_timer_add(&timer, job);
		}
	}
	while (1) {
		cdk_time_sleep(1000);
	}
	mtx_destroy(&mtx);
	cdk_timer_destroy(&timer);
	cdk_logger_destroy();
	return 0;
}