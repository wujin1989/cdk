#include "cdk.h"

int main(void) {
	cdk_logger_create(NULL, true);
	cdk_logi("logging\n");
	cdk_logger_destroy();
	return 0;
}