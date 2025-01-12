#/bin/sh

OHOS_NDK_HOME=${HOME}/AppData/Local/OpenHarmony/Sdk/12/native
export PATH=${OHOS_NDK_HOME}/build-tools/cmake/bin:$PATH
set CMAKE_SYSTEM_NAME="Harmony"
set CMAKE_SYSTEM_PROCESSOR="aarch64"

cmake.exe 																		\
	-G Ninja 																	\
	-B build 																	\
	-DCMAKE_TOOLCHAIN_FILE="${OHOS_NDK_HOME}/build/cmake/ohos.toolchain.cmake" 	\
	-DOHOS_ARCH=arm64-v8a														\
	-DOHOS_STL=c++_static														\
	
cmake.exe --build build
