cmake_minimum_required(VERSION 3.25)

project(cdk VERSION 1.0.0 LANGUAGES C)

if(WIN32 OR LINUX OR (APPLE AND NOT IOS) OR IOS OR ANDROID)
    message(STATUS "[cdk compile]: operation system is ${CMAKE_SYSTEM}")
else()
    message(FATAL_ERROR "[cdk compile]: operation system type is ${CMAKE_SYSTEM_NAME}, now not support")
endif()

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
	message(STATUS "[cdk compile]: current os is 64bit")
else()
	message(FATAL_ERROR "[cdk compile]: 32bit not supported")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

#iso c standard, support c11 and later
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_STANDARD 11)

if(WIN32)
add_compile_options("/experimental:c11atomics")
add_compile_options("/D_CRT_SECURE_NO_WARNINGS")
endif()

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

message(STATUS "[cdk compile]: using iso c${CMAKE_C_STANDARD} standard.")

include_directories(include)
include_directories(src)
include_directories(third-party/openssl/include)
	
if (CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64")
	if(WIN32)
		link_directories(third-party/openssl/lib/amd64/windows)
	endif()
	if(LINUX)
		link_directories(third-party/openssl/lib/amd64/linux)
	endif()
endif()

if (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
endif()

set(SRCS
	src/crypto/cdk-sha1.c
	src/crypto/cdk-sha256.c
	src/encoding/cdk-varint.c
	src/encoding/cdk-base64.c
	src/encoding/cdk-json.c
	src/container/cdk-list.c
	src/container/cdk-queue.c
	src/container/cdk-stack.c
	src/container/cdk-rbtree.c
	src/container/cdk-ringbuffer.c
	src/sync/cdk-rwlock.c
	src/sync/cdk-spinlock.c
	src/net/cdk-net.c
	src/net/cdk-poller.c
	src/net/cdk-channel.c
	src/net/cdk-unpack.c
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

add_library(cdk STATIC ${SRCS})
target_link_libraries(cdk PUBLIC Threads::Threads ssl crypto)

if(WIN32)
	target_link_libraries(cdk PUBLIC winmm ws2_32)
endif()

if(UNIX)
	target_link_libraries(cdk PUBLIC ${CMAKE_DL_LIBS})
endif()

add_subdirectory(examples)