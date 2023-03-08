cmake_minimum_required(VERSION 3.0)

project(cdk VERSION 1.0.0 LANGUAGES C)

message(STATUS "[cdk compile]: operation system is ${CMAKE_SYSTEM}")

#only support windows, linux, darwin(mac os)
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    message(STATUS "[cdk compile]: operation system type is ${CMAKE_SYSTEM_NAME}")
elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
    message(STATUS "[cdk compile]: operation system type is ${CMAKE_SYSTEM_NAME}")
elseif (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    message(STATUS "[cdk compile]: operation system type is ${CMAKE_SYSTEM_NAME}")
else ()
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

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

message(STATUS "[cdk compile]: using iso c${CMAKE_C_STANDARD} standard.")

include_directories(include)
include_directories(src/platform)
	
set(SRCS
	src/cdk-timer.c
	src/cdk-atomic.c
	src/cdk-cnd.c
	src/cdk-file.c
	src/cdk-list.c
	src/cdk-loader.c
	src/cdk-logger.c
	src/cdk-math.c
	src/cdk-memory.c
	src/cdk-mtx.c
#	src/cdk-net.c
	src/cdk-process.c
	src/cdk-queue.c
	src/cdk-rbtree.c
	src/cdk-ringbuffer.c
	src/cdk-rwlock.c
	src/cdk-stack.c
	src/cdk-string.c
	src/cdk-sysinfo.c
	src/cdk-thread.c
	src/cdk-threadpool.c
	src/cdk-time.c
	src/cdk-varint.c
	src/cdk-sha256.c
	src/cdk-base64.c
)

if(WIN32)
	list(APPEND SRCS 
		src/platform/win/win-process.c
		src/platform/win/win-thread.c
		src/platform/win/win-time.c
		src/platform/win/win-mtx.c
		src/platform/win/win-cnd.c
		src/platform/win/win-atomic.c
#		src/platform/win/win-net.c
		src/platform/win/win-sysinfo.c
		src/platform/win/win-loader.c
		src/platform/win/win-file.c
#		src/platform/win/win-poller.c
		src/platform/win/win-string.c
		src/platform/win/win-rwlock.c
	)
endif()

if(UNIX)
	list(APPEND SRCS 
		src/platform/unix/unix-process.c
		src/platform/unix/unix-thread.c
		src/platform/unix/unix-time.c
		src/platform/unix/unix-mtx.c
		src/platform/unix/unix-cnd.c
		src/platform/unix/unix-atomic.c
#		src/platform/unix/unix-net.c
		src/platform/unix/unix-sysinfo.c
		src/platform/unix/unix-loader.c
		src/platform/unix/unix-file.c
#		src/platform/unix/unix-poller.c
#		src/platform/unix/unix-event.c
		src/platform/unix/unix-string.c
		src/platform/unix/unix-rwlock.c
	)
endif()

add_library(cdk STATIC ${SRCS})
target_link_libraries(cdk PUBLIC Threads::Threads ${CMAKE_DL_LIBS})

add_subdirectory(examples)