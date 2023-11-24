cmake_minimum_required(VERSION 3.0)

project(examples LANGUAGES C)

add_executable(example-tcpserver "example-tcpserver.c")
target_link_libraries(example-tcpserver PUBLIC cdk)

add_executable(example-rwlock "example-rwlock.c")
target_link_libraries(example-rwlock PUBLIC cdk)

add_executable(example-spinlock "example-spinlock.c")
target_link_libraries(example-spinlock PUBLIC cdk)

add_executable(example-tcpclient "example-tcpclient.c")
target_link_libraries(example-tcpclient PUBLIC cdk)

add_executable(example-udpclient "example-udpclient.c")
target_link_libraries(example-udpclient PUBLIC cdk)

add_executable(example-udpserver "example-udpserver.c")
target_link_libraries(example-udpserver PUBLIC cdk)

add_executable(example-rbtree "example-rbtree.c")
target_link_libraries(example-rbtree PUBLIC cdk)

add_executable(example-varint "example-varint.c")
target_link_libraries(example-varint PUBLIC cdk)

add_executable(example-logger "example-logger.c")
target_link_libraries(example-logger PUBLIC cdk)

add_executable(example-timer "example-timer.c")
target_link_libraries(example-timer PUBLIC cdk)

add_executable(example-sha256 "example-sha256.c")
target_link_libraries(example-sha256 PUBLIC cdk)

add_executable(example-base64 "example-base64.c")
target_link_libraries(example-base64 PUBLIC cdk)

add_executable(example-sha1 "example-sha1.c")
target_link_libraries(example-sha1 PUBLIC cdk)

add_executable(example-mtx "example-mtx.c")
target_link_libraries(example-mtx PUBLIC cdk)

add_executable(example-ringbuf "example-ringbuf.c")
target_link_libraries(example-ringbuf PUBLIC cdk)