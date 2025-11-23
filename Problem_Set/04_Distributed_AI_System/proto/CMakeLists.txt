cmake_minimum_required(VERSION 3.8)

# Get Protobuf and gRPC executables
get_target_property(PROTOC_EXECUTABLE protobuf::protoc IMPORTED_LOCATION_RELEASE)
if(NOT PROTOC_EXECUTABLE)
    get_target_property(PROTOC_EXECUTABLE protobuf::protoc IMPORTED_LOCATION_DEBUG)
endif()

find_program(GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
if(NOT GRPC_CPP_PLUGIN_EXECUTABLE)
    get_target_property(GRPC_CPP_PLUGIN_EXECUTABLE gRPC::grpc_cpp_plugin IMPORTED_LOCATION_RELEASE)
    if(NOT GRPC_CPP_PLUGIN_EXECUTABLE)
     get_target_property(GRPC_CPP_PLUGIN_EXECUTABLE gRPC::grpc_cpp_plugin IMPORTED_LOCATION_DEBUG)
    endif()
endif()

set(PROTO_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY ${PROTO_GENERATED_DIR})

# Find all .proto files in the proto directory
file(GLOB PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.proto")

# Lists to hold generated files
set(PROTO_SRCS)
set(PROTO_HDRS)
set(GRPC_SRCS)
set(GRPC_HDRS)

# Generate protobuf and gRPC files for each .proto file
foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    
    set(PROTO_SRC "${PROTO_GENERATED_DIR}/${PROTO_NAME}.pb.cc")
    set(PROTO_HDR "${PROTO_GENERATED_DIR}/${PROTO_NAME}.pb.h")
    set(GRPC_SRC "${PROTO_GENERATED_DIR}/${PROTO_NAME}.grpc.pb.cc")
    set(GRPC_HDR "${PROTO_GENERATED_DIR}/${PROTO_NAME}.grpc.pb.h")
    
    list(APPEND PROTO_SRCS ${PROTO_SRC})
    list(APPEND PROTO_HDRS ${PROTO_HDR})
    list(APPEND GRPC_SRCS ${GRPC_SRC})
    list(APPEND GRPC_HDRS ${GRPC_HDR})
    
    # Custom command for protobuf generation
    add_custom_command(
        OUTPUT ${PROTO_SRC} ${PROTO_HDR}
        COMMAND ${PROTOC_EXECUTABLE}
        ARGS --cpp_out=${PROTO_GENERATED_DIR}
        -I${CMAKE_CURRENT_SOURCE_DIR}
        ${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
        COMMENT "Generating protobuf files for ${PROTO_NAME}.proto"
        VERBATIM
    )
    
    # Custom command for gRPC generation
    add_custom_command(
        OUTPUT ${GRPC_SRC} ${GRPC_HDR}
        COMMAND ${PROTOC_EXECUTABLE}
        ARGS --grpc_out=${PROTO_GENERATED_DIR}
        --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXECUTABLE}
        -I${CMAKE_CURRENT_SOURCE_DIR}
        ${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
        COMMENT "Generating gRPC files for ${PROTO_NAME}.proto"
        VERBATIM
    )
endforeach()

add_library(proto_library STATIC 
    ${PROTO_SRCS} 
    ${PROTO_HDRS}
    ${GRPC_SRCS} 
    ${GRPC_HDRS}
)

target_link_libraries(proto_library PUBLIC
    protobuf::libprotobuf
    gRPC::grpc++
)

target_include_directories(proto_library PUBLIC
    ${PROTO_GENERATED_DIR}
)

set_target_properties(proto_library PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

set(PROTO_GENERATED_DIR ${PROTO_GENERATED_DIR} PARENT_SCOPE)
