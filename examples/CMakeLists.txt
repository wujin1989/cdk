cmake_minimum_required(VERSION 3.10)

project(examples LANGUAGES C)


if (CMAKE_SIZEOF_VOID_P EQUAL 8)
	message(STATUS "[examples compile]: current os is 64bit!")
else()
	message(FATAL_ERROR "[examples compile]: 32bit os isn't supported!")
endif()

#iso c standard, support c11 and later
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_STANDARD 11)

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

include_directories(../include)
link_directories(../build/Debug)

add_executable(chat-client chat-client.c chat-protocol.h)
target_link_libraries(chat-client PUBLIC cdk Threads::Threads)

add_executable(chat-server chat-server.c chat-protocol.h)
target_link_libraries(chat-server PUBLIC cdk Threads::Threads)
