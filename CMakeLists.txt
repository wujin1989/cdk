cmake_minimum_required(VERSION 3.16)

project(cdk VERSION 1.0.0 LANGUAGES C)

option(BUILD_DYNAMIC_LIBRARY "build the dynamic library" OFF)
option(CMAKE_EXPORT_COMPILE_COMMANDS "export compile cmds" OFF)

if (CMAKE_SIZEOF_VOID_P LESS 8)
	message(FATAL_ERROR "only 64bit supported")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_STANDARD 11)

if(WIN32)
	if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
		add_compile_options("/experimental:c11atomics")
	endif()
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

set(SRCS
	src/crypto/cdk-sha1.c
	src/crypto/cdk-sha256.c
	src/encoding/cdk-varint.c
	src/encoding/cdk-base64.c
	src/container/cdk-list.c
	src/container/cdk-queue.c
	src/container/cdk-stack.c
	src/container/cdk-rbtree.c
	src/container/cdk-heap.c
	src/container/cdk-ringbuffer.c
	src/sync/cdk-rwlock.c
	src/sync/cdk-spinlock.c
	src/net/cdk-net.c
	src/net/poller.c
	src/net/channel.c
	src/net/unpacker.c
	src/net/tls.c
	src/net/txlist.c
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
		src/platform/win/platform-io.c
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
		src/platform/unix/platform-io.c
	)
endif()

if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64")
	set(CMAKE_HOST_SYSTEM_PROCESSOR "AMD64")
endif()

add_library(ssl IMPORTED STATIC)
add_library(crypto IMPORTED STATIC)

if(WIN32)
	set_target_properties(ssl PROPERTIES
		IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/third-party/openssl3/lib/${CMAKE_SYSTEM_PROCESSOR}/${CMAKE_SYSTEM_NAME}/ssl.lib"
		INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/third-party/openssl3//include")

	set_target_properties(crypto PROPERTIES
		IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/third-party/openssl3/lib/${CMAKE_SYSTEM_PROCESSOR}/${CMAKE_SYSTEM_NAME}/crypto.lib"
		INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/third-party/openssl3//include" 
		INTERFACE_LINK_LIBRARIES "Crypt32.lib")
else()
	set_target_properties(ssl PROPERTIES
		IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/third-party/openssl3/lib/${CMAKE_SYSTEM_PROCESSOR}/${CMAKE_SYSTEM_NAME}/libssl.a"
		INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/third-party/openssl3//include")

	set_target_properties(crypto PROPERTIES
		IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/third-party/openssl3/lib/${CMAKE_SYSTEM_PROCESSOR}/${CMAKE_SYSTEM_NAME}/libcrypto.a"
		INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/third-party/openssl3//include")
endif()

if(BUILD_DYNAMIC_LIBRARY)
    add_library(cdk SHARED ${SRCS})
	target_link_libraries(cdk PRIVATE ssl crypto ${CMAKE_DL_LIBS})
	set_target_properties(cdk PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS 1)
else()
    add_library(cdk STATIC ${SRCS})
	target_link_libraries(cdk PRIVATE ssl crypto ${CMAKE_DL_LIBS})
endif()

add_subdirectory(examples)
