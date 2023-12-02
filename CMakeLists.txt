cmake_minimum_required(VERSION 3.16)

project(cdk VERSION 1.0.0 LANGUAGES C)

option(BUILD_DYNAMIC_LIBRARY "build the dynamic library" OFF)

if (CMAKE_SIZEOF_VOID_P LESS 8)
	message(FATAL_ERROR "only 64bit supported")
endif()

if(WIN32)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_STANDARD 11)

if(WIN32)
add_compile_options("/experimental:c11atomics")
add_compile_options("/D_CRT_SECURE_NO_WARNINGS")
endif()

if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "AMD64" OR CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64")
	if (CMAKE_SYSTEM_NAME MATCHES "Windows")
		set(OPENSSL_ROOT_DIR "third-party/openssl3/amd64/win32")
	endif()
	if (CMAKE_SYSTEM_NAME MATCHES "Linux")
		set(OPENSSL_ROOT_DIR "third-party/openssl3/amd64/linux")
	endif()
	if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
		set(OPENSSL_ROOT_DIR "third-party/openssl3/amd64/macos")
	endif()
	if (CMAKE_SYSTEM_NAME MATCHES "Android")
		set(OPENSSL_ROOT_DIR "third-party/openssl3/amd64/android")
	endif()
	if (CMAKE_SYSTEM_NAME MATCHES "iOS")
		set(OPENSSL_ROOT_DIR "third-party/openssl3/amd64/ios")
	endif()
endif()		

if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "aarch64")
	if (CMAKE_SYSTEM_NAME MATCHES "Windows")
		set(OPENSSL_ROOT_DIR "third-party/openssl3/aarch64/win32")
	endif()
	if (CMAKE_SYSTEM_NAME MATCHES "Linux")
		set(OPENSSL_ROOT_DIR "third-party/openssl3/aarch64/linux")
	endif()
	if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
		set(OPENSSL_ROOT_DIR "third-party/openssl3/aarch64/macos")
	endif()
	if (CMAKE_SYSTEM_NAME MATCHES "Android")
		set(OPENSSL_ROOT_DIR "third-party/openssl3/aarch64/android")
	endif()
	if (CMAKE_SYSTEM_NAME MATCHES "iOS")
		set(OPENSSL_ROOT_DIR "third-party/openssl3/aarch64/ios")
	endif()
endif()

find_package(OpenSSL REQUIRED)

include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(include)
include_directories(src)


set(SRCS
	src/crypto/cdk-sha1.c
	src/crypto/cdk-sha256.c
	src/encoding/cdk-varint.c
	src/encoding/cdk-base64.c
	src/container/cdk-list.c
	src/container/cdk-queue.c
	src/container/cdk-stack.c
	src/container/cdk-rbtree.c
	src/container/cdk-ringbuffer.c
	src/sync/cdk-rwlock.c
	src/sync/cdk-spinlock.c
	src/net/cdk-net.c
	src/net/poller.c
	src/net/channel.c
	src/net/unpack.c
	src/net/tls.c
	src/cdk-threadpool.c
	src/cdk-loader.c
	src/cdk-logger.c
	src/cdk-process.c
	src/cdk-utils.c
	src/cdk-time.c
	src/cdk-timer.c
)

if(WIN32)
	list(APPEND SRCS 
		src/platform/win/platform-process.c
		src/platform/win/platform-time.c
		src/platform/win/platform-socket.c
		src/platform/win/platform-utils.c
		src/platform/win/platform-loader.c
		src/platform/win/wepoll/wepoll.c
		src/platform/win/platform-event.c
	)
endif()

if(UNIX)
	list(APPEND SRCS 
		src/platform/unix/platform-process.c
		src/platform/unix/platform-time.c
		src/platform/unix/platform-socket.c
		src/platform/unix/platform-utils.c
		src/platform/unix/platform-loader.c
		src/platform/unix/platform-event.c
	)
endif()

if(BUILD_DYNAMIC_LIBRARY)
    add_library(cdk SHARED ${SRCS})
	target_link_libraries(cdk PUBLIC ${OPENSSL_LIBRARIES})
	set_target_properties(cdk PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS 1)
else()
    add_library(cdk STATIC ${SRCS})
	target_link_libraries(cdk PUBLIC ${OPENSSL_LIBRARIES})
endif()

add_subdirectory(examples)
