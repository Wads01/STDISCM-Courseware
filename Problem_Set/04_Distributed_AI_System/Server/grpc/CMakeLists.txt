cmake_minimum_required(VERSION 3.8)

add_library(grpc_server STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/OCRServiceImpl.cpp
)

target_include_directories(grpc_server PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/proto/generated
)

target_link_libraries(grpc_server PUBLIC
    ocrLib
    proto_library
    gRPC::grpc++
    protobuf::libprotobuf
    ${OpenCV_LIBS}
)

set_target_properties(grpc_server PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
