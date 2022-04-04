cmake_minimum_required(VERSION 3.20)

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
	include/cdk/cdk-memory.h)
	
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
	src/cdk-memory.c)
	
if(WIN32)
	
	list(APPEND SRCS 
		src/win/win-process.c
		src/win/win-thread.c
		src/win/win-time.c
		src/win/win-sync.c
		src/win/win-atomic.c
		src/win/win-net.c
		src/win/win-systeminfo.c
		src/win/win-io.c
		src/win/win-process.h
		src/win/win-thread.h
		src/win/win-time.h
		src/win/win-sync.h
		src/win/win-atomic.h
		src/win/win-net.h
		src/win/win-systeminfo.h
		src/win/win-io.h)
endif()

if(UNIX)
	
	list(APPEND SRCS 
		src/unix/unix-process.c
		src/unix/unix-thread.c
		src/unix/unix-time.c
		src/unix/unix-sync.c
		src/unix/unix-atomic.c
		src/unix/unix-net.c
		src/unix/unix-systeminfo.c
		src/unix/unix-io.c
		src/unix/unix-process.h
		src/unix/unix-thread.h
		src/unix/unix-time.h
		src/unix/unix-sync.h
		src/unix/unix-atomic.h
		src/unix/unix-net.h
		src/unix/unix-systeminfo.h
		src/unix/unix-io.h)
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HDRS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})

add_library(cdk STATIC ${SRCS} ${HDRS})
