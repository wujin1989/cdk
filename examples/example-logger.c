#include "cdk.h"

int main(void) 
{
	cdk_logger_create(NULL, 4);
	cdk_logi("logging\n");
	cdk_logger_destroy();
	return 0;
}