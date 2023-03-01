cmake_minimum_required(VERSION 3.16)

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
	message(STATUS "[cdk compile]: current os is 64bit!")
else()
	message(FATAL_ERROR "[cdk compile]: 32bit os isn't supported!")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

#iso c standard, support c11 and later
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_STANDARD 11)

message(STATUS "[cdk compile]: using iso c${CMAKE_C_STANDARD} standard.")

include_directories(include)

set(HDRS 
	include/cdk.h
	include/cdk/cdk-logger.h
	include/cdk/cdk-prng.h
	include/cdk/cdk-process.h
	include/cdk/cdk-thread.h
	include/cdk/cdk-time.h
	include/cdk/cdk-types.h
	include/cdk/cdk-sync.h
	include/cdk/cdk-atomic.h
	include/cdk/cdk-net.h
	include/cdk/cdk-queue.h
	include/cdk/cdk-stack.h
	include/cdk/cdk-list.h
	include/cdk/cdk-threadpool.h
	include/cdk/cdk-systeminfo.h
	include/cdk/cdk-io.h
	include/cdk/cdk-memory.h
	include/cdk/cdk-loadapi.h
	include/cdk/cdk-rbtree.h
	include/cdk/cdk-ringbuffer.h
	include/cdk/cdk-varint.h)
	
set(SRCS 
	src/cdk-logger.c
	src/cdk-prng.c
	src/cdk-process.c
	src/cdk-thread.c
	src/cdk-time.c
	src/cdk-sync.c
	src/cdk-atomic.c
	src/cdk-net.c
	src/cdk-queue.c
	src/cdk-stack.c
	src/cdk-list.c
	src/cdk-threadpool.c
	src/cdk-systeminfo.c
	src/cdk-io.c
	src/cdk-memory.c
	src/cdk-loadapi.c
	src/cdk-rbtree.c
	src/cdk-ringbuffer.c
	src/cdk-varint.c
	src/platform/platform-process.h
	src/platform/platform-atomic.h
	src/platform/platform-event.h
	src/platform/platform-io.h
	src/platform/platform-loadapi.h
	src/platform/platform-net.h
	src/platform/platform-poller.h
	src/platform/platform-sync.h
	src/platform/platform-systeminfo.h
	src/platform/platform-thread.h
	src/platform/platform-time.h
	)
	
if(WIN32)
	list(APPEND SRCS 
		src/platform/win/win-process.c
		src/platform/win/win-thread.c
		src/platform/win/win-time.c
		src/platform/win/win-sync.c
		src/platform/win/win-atomic.c
		src/platform/win/win-net.c
		src/platform/win/win-systeminfo.c
		src/platform/win/win-io.c
		src/platform/win/win-loadapi.c
		src/platform/win/win-poller.c
		)
endif()

if(UNIX)
	list(APPEND SRCS 
		src/platform/unix/unix-process.c
		src/platform/unix/unix-thread.c
		src/platform/unix/unix-time.c
		src/platform/unix/unix-sync.c
		src/platform/unix/unix-atomic.c
		src/platform/unix/unix-net.c
		src/platform/unix/unix-systeminfo.c
		src/platform/unix/unix-io.c
		src/platform/unix/unix-loadapi.c
		src/platform/unix/unix-poller.c
		src/platform/unix/unix-event.c
		)
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HDRS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})

add_library(cdk STATIC ${SRCS} ${HDRS})

add_subdirectory(examples)