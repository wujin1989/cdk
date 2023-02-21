#include "cdk.h"




typedef struct _data_t {
	mtx_t mutex;
	int   value;
}data_t;

void func(data_t* data) {

	//cdk_mtx_unlock(&data->mutex);
	cdk_mtx_destroy(&data->mutex);
}
int main(void) {

	data_t* data = cdk_malloc(sizeof(data_t));
	
	data->value = 2;
	cdk_mtx_init(&data->mutex);
	cdk_mtx_lock(&data->mutex);
	func(data);


	return 0;
}