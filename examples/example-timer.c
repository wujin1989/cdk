#include "cdk.h"

static inline void _timer_task(void* p) {
	cdk_logi("[%d]timer task\n", (int)cdk_utils_systemtid());
}

int main(void) {
	cdk_timermgr_t* timermgr;
	cdk_logger_config_t config = {
		.async = false,
	};
	cdk_logger_create(&config);
    timermgr = cdk_timer_manager_create();
	
	cdk_timer_add(timermgr, _timer_task, NULL, 1000, true);
	while (true) {
        if (cdk_heap_empty(&timermgr->heap)) {
			break;
		}
        cdk_timer_t* min =
            cdk_heap_data(cdk_heap_min(&timermgr->heap), cdk_timer_t, node);
		
		if (min->birth + min->expire > cdk_time_now()) {
			cdk_time_sleep(min->birth + min->expire - cdk_time_now());
			continue;
		}
		min->routine(min->param);
		if (min->repeat) {
            cdk_timer_reset(timermgr, min, min->expire);
		} else {
            cdk_timer_del(timermgr, min);
		}
	}
    cdk_timer_manager_destroy(timermgr);
	cdk_logger_destroy();
	return 0;
}