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

add_executable(chat-client chat-client.c chat-protocol.h)
target_link_libraries(chat-client PUBLIC cdk Threads::Threads)

add_executable(chat-server chat-server.c chat-protocol.h)
target_link_libraries(chat-server PUBLIC cdk Threads::Threads)


if(UNIX)
add_executable(multi-threads-share-same-epfd "multi-threads-share-same-epfd.c")
target_link_libraries(multi-threads-share-same-epfd PUBLIC cdk Threads::Threads)

add_executable(multi-threads-reuse-port "multi-threads-reuse-port.c")
target_link_libraries(multi-threads-reuse-port PUBLIC cdk Threads::Threads)

add_executable(multi-epfds-share-same-sockfd "multi-epfds-share-same-sockfd.c")
target_link_libraries(multi-epfds-share-same-sockfd PUBLIC cdk Threads::Threads)

add_executable(eventfd-usage "eventfd-usage.c")
target_link_libraries(eventfd-usage PUBLIC cdk Threads::Threads)

add_executable(epoll-usage "epoll-usage.c")
target_link_libraries(epoll-usage PUBLIC cdk Threads::Threads)
endif()

if(WIN32)
add_executable(iocp-usage "iocp-usage.c")
target_link_libraries(iocp-usage PUBLIC cdk Threads::Threads)
endif()