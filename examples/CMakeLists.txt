cmake_minimum_required(VERSION 3.0)

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

#add_executable(chat-client chat-client.c chat-protocol.h)
#target_link_libraries(chat-client PUBLIC cdk Threads::Threads)

#add_executable(chat-server chat-server.c chat-protocol.h)
#target_link_libraries(chat-server PUBLIC cdk Threads::Threads)


if(UNIX)
#add_executable(multi-threads-share-same-epfd "multi-threads-share-same-epfd.c")
#target_link_libraries(multi-threads-share-same-epfd PUBLIC cdk Threads::Threads)

#add_executable(multi-threads-reuse-port "multi-threads-reuse-port.c")
#target_link_libraries(multi-threads-reuse-port PUBLIC cdk Threads::Threads)

#add_executable(multi-epfds-share-same-sockfd "multi-epfds-share-same-sockfd.c")
#target_link_libraries(multi-epfds-share-same-sockfd PUBLIC cdk Threads::Threads)

#add_executable(eventfd-usage "eventfd-usage.c")
#target_link_libraries(eventfd-usage PUBLIC cdk Threads::Threads)

#add_executable(epoll-usage "epoll-usage.c")
#target_link_libraries(epoll-usage PUBLIC cdk Threads::Threads)
endif()

if(WIN32)
#add_executable(iocp-usage "iocp-usage.c")
#target_link_libraries(iocp-usage PUBLIC cdk Threads::Threads)
endif()

#add_executable(cdk-asyn-net "cdk-asyn-net.c")
#target_link_libraries(cdk-asyn-net PUBLIC cdk Threads::Threads)

#add_executable(list-usage "list-usage.c")
#target_link_libraries(list-usage PUBLIC cdk Threads::Threads)

#add_executable(cdk-asyn-net-client "cdk-asyn-net-client.c")
#target_link_libraries(cdk-asyn-net-client PUBLIC cdk Threads::Threads)

#add_executable(udp-client "udp-client.c")
#target_link_libraries(udp-client PUBLIC cdk Threads::Threads)

#add_executable(udp-server "udp-server.c")
#target_link_libraries(udp-server PUBLIC cdk Threads::Threads)

add_executable(load-library "file/load-library.c")
target_link_libraries(load-library PUBLIC cdk)

add_executable(test-rbtree "test-rbtree.c")
target_link_libraries(test-rbtree PUBLIC cdk)

add_executable(test-varint "test-varint.c")
target_link_libraries(test-varint PUBLIC cdk)

add_executable(test-logger "test-logger.c")
target_link_libraries(test-logger PUBLIC cdk)

add_executable(test-timer "test-timer.c")
target_link_libraries(test-timer PUBLIC cdk)

add_executable(test-sha256 "test-sha256.c")
target_link_libraries(test-sha256 PUBLIC cdk)

add_executable(test-base64 "test-base64.c")
target_link_libraries(test-base64 PUBLIC cdk)

add_executable(test-sha1 "test-sha1.c")
target_link_libraries(test-sha1 PUBLIC cdk)