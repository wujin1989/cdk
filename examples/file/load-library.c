#include "cdk.h"
#include <stdio.h>

#if defined(__linux__) || defined(__APPLE__)
#define PATH "../../../../src/examples/file/load-library-libs/libcalc.so"
#endif

#if defined(_WIN32)
#define PATH "../../../../examples/file/load-library-libs/calc.dll"
#endif

typedef int (*fn_t)(int, int);
int main(void) {

	void* handle;
	fn_t  fn;
	handle = cdk_loader_create(PATH);

	fn = (fn_t)cdk_loader_load(handle, "add");
	
	printf("result: %d\n", fn(1, 2));
	cdk_loader_destroy(handle);
	return 0;
}