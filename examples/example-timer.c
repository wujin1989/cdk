#include "cdk.h"

mtx_t mtx;
static inline void _routine_cb(void* p) {
	mtx_lock(&mtx);
	cdk_logi("[%d]timer task\n", (int)cdk_utils_systemtid());
	mtx_unlock(&mtx);
}

int main(void) {
	cdk_timermgr_t mgr;
	cdk_timer_t* timer;
	
	cdk_logger_config_t config = {
		.async = false,
	};
	cdk_logger_create(&config);
	cdk_timer_manager_init(&mgr);
	mtx_init(&mtx, mtx_plain);
	
	timer = cdk_timer_add(&mgr, _routine_cb, NULL, 1000, true);
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