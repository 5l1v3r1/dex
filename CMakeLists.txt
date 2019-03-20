project(grpc-cmake-example)
cmake_minimum_required(VERSION 3.2)

add_compile_options(-std=c++14)

include(FindPkgConfig)

# GRPC and Protocol Buffers libraries location
list(APPEND CMAKE_PREFIX_PATH "/opt/grpc" "/opt/protobuf" "/opt/cpp_redis")

# Cmake find modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(Protobuf REQUIRED)
find_package(GRPC REQUIRED)
find_package(PkgConfig REQUIRED)

set(PROTOS
    ${CMAKE_CURRENT_SOURCE_DIR}/protos/order.proto
)

set(PROTO_SRC_DIR ${CMAKE_CURRENT_BINARY_DIR}/proto-src)
file(MAKE_DIRECTORY ${PROTO_SRC_DIR})
include_directories(${PROTO_SRC_DIR})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_SRC_DIR} ${PROTOS})
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${PROTO_SRC_DIR} ${PROTOS})

pkg_check_modules(CPP_REDIS REQUIRED cpp_redis)

set(ADDITIONAL_LINKER_FLAGS "-ltacopie")

# Building order server
add_executable(order_server
    ${CMAKE_CURRENT_SOURCE_DIR}/src/order_server.cpp
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)
target_link_libraries(order_server
    gRPC::grpc++_reflection
    protobuf::libprotobuf
    ${CPP_REDIS_LIBRARIES}
    ${ADDITIONAL_LINKER_FLAGS}
)

# Building order client
add_executable(order_client
    ${CMAKE_CURRENT_SOURCE_DIR}/src/order_client.cpp
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)
target_link_libraries(order_client
    gRPC::grpc++_reflection
    protobuf::libprotobuf
)

# Building exchange server
add_executable(exchange_server
    ${CMAKE_CURRENT_SOURCE_DIR}/src/exchange_server.cpp
)
target_link_libraries(exchange_server
)
