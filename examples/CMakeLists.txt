cmake_minimum_required(VERSION 3.16)

project(examples LANGUAGES C)

add_executable(example-sync-tcp-client "example-sync-tcp-client.c")
target_link_libraries(example-sync-tcp-client PUBLIC cdk)

add_executable(example-sync-tcp-server "example-sync-tcp-server.c")
target_link_libraries(example-sync-tcp-server PUBLIC cdk)

add_executable(example-tcp-perf-client "example-tcp-perf-client.c")
target_link_libraries(example-tcp-perf-client PUBLIC cdk)

add_executable(example-tcp-perf-server "example-tcp-perf-server.c")
target_link_libraries(example-tcp-perf-server PUBLIC cdk)

add_executable(example-async-tcp-server "example-async-tcp-server.c")
target_link_libraries(example-async-tcp-server PUBLIC cdk)

add_executable(example-async-tcp-client "example-async-tcp-client.c")
target_link_libraries(example-async-tcp-client PUBLIC cdk)

add_executable(example-udp-client "example-udp-client.c")
target_link_libraries(example-udp-client PUBLIC cdk)

add_executable(example-udp-server "example-udp-server.c")
target_link_libraries(example-udp-server PUBLIC cdk)

add_executable(example-rbtree "example-rbtree.c")
target_link_libraries(example-rbtree PUBLIC cdk)

add_executable(example-varint "example-varint.c")
target_link_libraries(example-varint PUBLIC cdk)

add_executable(example-sha256 "example-sha256.c")
target_link_libraries(example-sha256 PUBLIC cdk)

add_executable(example-base64 "example-base64.c")
target_link_libraries(example-base64 PUBLIC cdk)

add_executable(example-sha1 "example-sha1.c")
target_link_libraries(example-sha1 PUBLIC cdk)

add_executable(example-ringbuf "example-ringbuf.c")
target_link_libraries(example-ringbuf PUBLIC cdk)

add_executable(example-heap "example-heap.c")
target_link_libraries(example-heap PUBLIC cdk)

add_executable(example-timer "example-timer.c")
target_link_libraries(example-timer PUBLIC cdk)

install(TARGETS example-sync-tcp-client DESTINATION bin)
install(TARGETS example-sync-tcp-server DESTINATION bin)
install(TARGETS example-tcp-perf-client DESTINATION bin)
install(TARGETS example-tcp-perf-server DESTINATION bin)
install(TARGETS example-async-tcp-client DESTINATION bin)
install(TARGETS example-async-tcp-server DESTINATION bin)
install(TARGETS example-udp-client DESTINATION bin)
install(TARGETS example-udp-server DESTINATION bin)
install(TARGETS example-rbtree DESTINATION bin)
install(TARGETS example-varint DESTINATION bin)
install(TARGETS example-sha256 DESTINATION bin)
install(TARGETS example-base64 DESTINATION bin)
install(TARGETS example-sha1 DESTINATION bin)
install(TARGETS example-ringbuf DESTINATION bin)
install(TARGETS example-heap DESTINATION bin)
install(TARGETS example-timer DESTINATION bin)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/certs DESTINATION bin FILES_MATCHING PATTERN "*")