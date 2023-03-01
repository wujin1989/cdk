#include "cdk.h"




typedef struct _data_t {
	mtx_t mutex;
	int   value;
}data_t;

void func(data_t* data) {

	//cdk_mtx_unlock(&data->mutex);
	cdk_mtx_destroy(&data->mutex);
}

int task(void* p) {

	pthread_t tid = pthread_self();
	printf("task: %ul\n", tid);
	return 0;
}
int main(void) {

	cdk_logger_init(NULL, false);
	/*data_t* data = cdk_malloc(sizeof(data_t));
	
	data->value = 2;
	cdk_mtx_init(&data->mutex);
	cdk_mtx_lock(&data->mutex);
	func(data);*/
	thrd_t td;
	cdk_thrd_create(&td, task, NULL);
	pthread_t tid = pthread_self();
	printf("main: %ul\n", tid);
	return 0;
}