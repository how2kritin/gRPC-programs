cmake_minimum_required(VERSION 3.29)
project(DS_HW4_Team-32)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
pkg_search_module(UUID REQUIRED uuid)
get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
get_target_property(grpc_py_plugin_location gRPC::grpc_python_plugin LOCATION)

# protolibs
add_library(KNN_protolib KNN/protofiles/q2.proto)
target_link_libraries(KNN_protolib gRPC::grpc++)
target_include_directories(KNN_protolib PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
protobuf_generate(TARGET KNN_protolib LANGUAGE cpp)
protobuf_generate(TARGET KNN_protolib LANGUAGE grpc GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")

add_library(MyUber_protolib MyUber/protofiles/q3.proto)
target_link_libraries(MyUber_protolib gRPC::grpc++)
target_include_directories(MyUber_protolib PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
protobuf_generate(TARGET MyUber_protolib LANGUAGE cpp)
protobuf_generate(TARGET MyUber_protolib LANGUAGE grpc GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")

# client and server
add_executable(KNN_server KNN/server/server.cpp)
add_executable(KNN_client KNN/client/client.cpp)
add_executable(MyUber_client MyUber/client/main_handler_client.cpp MyUber/client/driver_client.cpp MyUber/client/rider_client.cpp)

target_link_libraries(KNN_server KNN_protolib)
target_link_libraries(KNN_client KNN_protolib)
target_link_libraries(MyUber_client MyUber_protolib)
target_link_libraries(MyUber_client  ${UUID_LIBRARIES})

