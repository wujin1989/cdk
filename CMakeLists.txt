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
include_directories(src/platform)
	


file(GLOB ATOMIC_SRCS    src/atomic/*.c)
file(GLOB CONTAINER_SRCS src/container/*.c)
file(GLOB ENCODING_SRCS  src/encoding/*.c)
file(GLOB FILE_SRCS      src/file/*.c)
file(GLOB IO_SRCS        src/io/*.c)
#file(GLOB LOGGER_SRCS   src/logger/*.c)
file(GLOB MATH_SRCS      src/math/*.c)
file(GLOB MEMORY_SRCS    src/memory/*.c)
#file(GLOB NET_SRCS      src/net/*.c)
file(GLOB PROCESS_SRCS   src/process/*.c)
file(GLOB STRING_SRCS    src/string/*.c)
#file(GLOB SYNC_SRCS     src/sync/*.c)
#file(GLOB SYSINFO_SRCS  src/sysinfo/*.c)
#file(GLOB THREAD_SRCS   src/thread/*.c)
#file(GLOB TIME_SRCS     src/time/*.c)

set(SRCS 
	${ATOMIC_SRCS} ${CONTAINER_SRCS} ${ENCODING_SRCS} ${FILE_SRCS} ${IO_SRCS} 
	${LOGGER_SRCS} ${MATH_SRCS} ${MEMORY_SRCS} ${PROCESS_SRCS} ${NET_SRCS} 
	${STRING_SRCS} ${SYNC_SRCS} ${SYSINFO_SRCS} ${THREAD_SRCS} ${TIME_SRCS}
)

if(WIN32)
	list(APPEND SRCS 
		src/platform/win/win-process.c
#		src/platform/win/win-thread.c
#		src/platform/win/win-time.c
#		src/platform/win/win-sync.c
		src/platform/win/win-atomic.c
#		src/platform/win/win-net.c
#		src/platform/win/win-systeminfo.c
		src/platform/win/win-io.c
		src/platform/win/win-loader.c
		src/platform/win/win-file.c
#		src/platform/win/win-poller.c
	)
endif()

if(UNIX)
	list(APPEND SRCS 
		src/platform/unix/unix-process.c
#		src/platform/unix/unix-thread.c
#		src/platform/unix/unix-time.c
#		src/platform/unix/unix-sync.c
		src/platform/unix/unix-atomic.c
#		src/platform/unix/unix-net.c
#		src/platform/unix/unix-systeminfo.c
		src/platform/unix/unix-io.c
		src/platform/unix/unix-loader.c
		src/platform/unix/unix-file.c
#		src/platform/unix/unix-poller.c
#		src/platform/unix/unix-event.c
	)
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HDRS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})

add_library(cdk STATIC ${SRCS} ${HDRS})

add_subdirectory(examples)