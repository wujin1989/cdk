#include "cdk.h"

mtx_t mtx;

void callback(void* p) {
	mtx_lock(&mtx);
	cdk_logi("[%d]timer task\n", (int)cdk_utils_systemtid());
	mtx_unlock(&mtx);
}

int main(void) {
	cdk_timermgr_t mgr;
	cdk_timer_t* timer;
	cdk_timer_manager_init(&mgr);
	cdk_logger_create(NULL, true);
	mtx_init(&mtx, mtx_plain);
	
	timer = cdk_timer_add(&mgr, callback, NULL, 1000, true);
	while (true) {
		if (cdk_heap_empty(&mgr.heap)) {
			break;
		}
		cdk_timer_t* min = cdk_heap_data(cdk_heap_min(&mgr.heap), cdk_timer_t, node);
		
		if (min->birth + min->expire > cdk_time_now()) {
			cdk_time_sleep(min->birth + min->expire - cdk_time_now());
			continue;
		}
		min->routine(min->param);
		if (min->repeat) {
			cdk_timer_reset(&mgr, min, min->expire);
		} else {
			cdk_timer_del(&mgr, min);
		}
	}
	mtx_destroy(&mtx);
	cdk_logger_destroy();
	return 0;
}